#include <iostream>
#include <getopt.h>
#include "Matrix/Matrix.h"
#include "Matrix/FastookReader.h"
#include "Communication/Thread.h"
#include "Communication/Message.h"
#include "Processing/JacobiCPU.h"
#include "Processing/JacobiGPU.h"
#include "Processing/Distributor.h"
#include "Processing/ResidualCalculator.h"
#include "Util/Timer.h"

using namespace std;
#define COMPLETE_THRESHOLD 5
//5

//#define ROW 301
//#define COL 561

#define Solver	JacobiGPU

const char* optString = "o:i:n:";

string defaultOutput = "output";

static const struct option longOpts[] = {
	{ "output", required_argument, NULL, 'o' },
	{ "input", required_argument, NULL, 'i' },
	{ "nProcs", required_argument, NULL, 'a' },
};

int main(int argc, char * argv[])
{
    int nProcs = 1;

	bool inputFlag = false;
	bool nProcsFlag = false;
	bool outputFlag = false;

	string outputFileName;
	string inputFileName;

	int longIndex;
	int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

	outputFileName = defaultOutput;

	while (opt != -1) {
		switch (opt) {
		case 'i':
			inputFlag = true;
			inputFileName = optarg;
			break;
		case 'o':
			outputFlag = true;
			outputFileName = optarg;
			break;
		case 'n':
			nProcsFlag = true;
			nProcs = atoi(optarg);
		}

		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	if (!inputFlag) {
		cout << "Error: Requires input file to operate on."  << endl;
		return -1;
	}

	Timer setup("Read");
	Timer comp("Computation");
	setup.start();
	// Produce some classes
	FastookReader reader(inputFileName.c_str());
//	FastookReader reader("testMatrix");
//	FastookReader reader("matrix.file");



	Solver* jacobiSolver = new Solver[nProcs];
	//Distributor distributor[nProcs];
	Mailbox* collection = new Mailbox[nProcs];
	Mailbox ***list = new Mailbox**[nProcs];//[nProcs+1];
    for (int i = 0; i < nProcs; ++i) {
        list[i] = new Mailbox*[nProcs + 1];
    }
	FILE *file;
	int* completeFlags = new int[nProcs];
	bool done = false;

	file = fopen(outputFileName.c_str(), "w");

	// Setup which distributor controls which thread
	// When PROC=1 these do essentially nothing
	for (int i = 0; i < nProcs; i++) {
		collection[i].setLocking(false);
		//distributor[i].setControl(list[i], &jacobiSolver[i], &collection);
	}

	// Matrix setup stuff
	reader.readFile();
	MatrixRow** rows[nProcs];
	int* rowInd[nProcs];
	int height = reader.getA()->getHeight();
	int size = height/nProcs + 1;
	ResidualCalculator res(reader.getA(), reader.getB(), reader.getX());

	// Allocate memory for row pointer storage
	// Setup distribution lists
	for (int i = 0; i < nProcs; i++) {
		rows[i] = new MatrixRow*[size];
		if (!rows[i]) {
			perror("Unable to Allocate Memory!");
			return 1;
		}
		rowInd[i] = new int[size];
		if (!rowInd[i]) {
			perror("Unable to Allocate Memory!");
			return 1;
		}
/*
		for (int j = 0; j < i; j++) {
			list[i][j] = &jacobiSolver[j].mailbox;
		}
*/
		for (int j = 0; j < nProcs-1; j++) {
			list[i][j] = &jacobiSolver[(j + i + 1) % nProcs].mailbox;
		}
		//list[i][nProcs-1] = &res.mailbox;
		list[i][nProcs-1] = NULL;
	}

	// Fill in row pointer memory
	// This distributes what nProcs solve which vars
	for (int j = 0; j < nProcs; j++) {
		for (int i = 0; i < size; i++) {
			if (j*size+i < height) {
				rowInd[j][i] = j*size+i;
				rows[j][i] = reader.getA()->getRow(j*size+i);
			} else {
				rowInd[j][i] = -1;
				rows[j][i] = NULL;
			}
		}
	}
	setup.stop();
	comp.start();

	// Finally give setup control data to jacobi nProcs
	// Also setup max number of iterations between comm
	// packets.
	for (int i = 0; i < nProcs; i++) {
		int j;
		for (j = 0; j < size; j++) {
			if (rows[i][j] == NULL) {
				break;
			}
		}

		jacobiSolver[i].setControl(list[i], collection+i, rows[i], reader.getB(), rowInd[i], j, height, i
                , nProcs
                );
		jacobiSolver[i].setMaxIter(5);
	}

	// Start all threads
	for (int i = 0; i < nProcs; i++) {
		//distributor[i].start();
		jacobiSolver[i].start();
	}
	//res.start();
	// Wait till everyone done.
	while (!done) {
		// Check for status messages
		for (int i = 0; i < nProcs; i++) {
			while (collection[i].hasMessage()) {
				Message message = collection[i].getMessage();
				if (message.getType() == ProcStatus) {
					int* buf = (int *)message.getData();
					if (buf[1]) {
						// Wait a little bit after completion to make sure all are really done
						// counting to some number solves this
						completeFlags[buf[0]]++;
//                        printf("Got Status Done %d\n", i);
					} else {
						completeFlags[buf[0]] = 0;
//                        printf("Got Status Undone %d\n", i);
					}
					done = true;
					for (int i = 0; i < nProcs; i++) {
						if (completeFlags[i] < COMPLETE_THRESHOLD) {
							done = false;
							break;
						}
					}
				} else {
					printf("Message %d\n", message.getType());
				}
			}
			// Give up some GPU time.
		}
		usleep(100);
	}
	// Stop the threads
	// Distributors will stop when owner completes
	printf("Stopping threads\n");
	for (int i = 0; i < nProcs; i++) {
		jacobiSolver[i].setRunning(false);
	}
	for (int i = 0; i < nProcs; i++) {
		jacobiSolver[i].stop();
	}
	res.mRun = false;
	comp.stop();
	//res.stop();
	// Print some results.
	for (int j = 0; j < nProcs; j++) {
		for (int i = 0; i < jacobiSolver[j].getSize(); i++) {
			fprintf(file, "%lf\n", jacobiSolver[j].getX()[i + j * (size)]);
		}
	}
//	for (int i = 0; i < nProcs; i++) {
//		if (jacobiSolver[i].isAwake()) {
//			jacobiSolver[i].stop();
//		}
//		if (distributor[i].isAwake()) {
//			distributor[i].stop();
//		}
//	}
	// Free some memory
	for (int i = 0; i < nProcs; i++) {
		delete[] rows[i];
		delete[] rowInd[i];
	}

	return 0;
}
/**
 * Test of one processor. Time:
real	0m1.003s
user	0m0.007s
sys	0m0.003s
 *
 */
//int main(int argc, char * argv[])
//{
//	FastookReader reader("matrix.file");
//	Solver jacobiSolver;
//
//	reader.readFile();
//	MatrixRow** rows = new MatrixRow*[reader.getA()->getHeight()];
//	int* rowInd = new int[reader.getA()->getHeight()];
//	for (int i = 0; i < reader.getA()->getHeight(); i++) {
//		rows[i] = reader.getA()->getRow(i);
//		rowInd[i] = i;
//	}
//
//	jacobiSolver.setControl(NULL, rows, reader.getB(), rowInd, reader.getA()->getHeight(), reader.getA()->getHeight());
//	jacobiSolver.setMaxIter(10);
//
//	jacobiSolver.start();
//	while (jacobiSolver.isRunning()) {
//		printf("Running\n");
//		sleep(1);
//	}
//	for (int i = 0; i < reader.getA()->getHeight(); i++) {
//		printf("%lf\n", jacobiSolver.getX()[i]);
//	}
//	delete[] rows;
//
//	return 0;
//}

/**
 * Old Matrix Test Code
 *
	FastookReader reader("matrix.file");
	Matrix A(3,3), x(3,1), b(3, 1);
	Matrix c(1, 1);
	double sum = 0;
	const void *p = "Hello Thread";
	Message message(StringMessage, p, 13);
	Thread thread;
	thread.mailbox.addMessage(message);
	thread.start();

	cout << "Starting Read..." << endl;
	thread.mailbox.addMessage(message);

	int count = 1;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			A.setVal(j, i, count++);
		}
		x.setVal(i, 0, i+1);
	}
	b.setVal(0, 0, 30);
	b.setVal(1, 0, 36);
	b.setVal(nProcs, 0, 4nProcs);

//	reader.readFile();
//	A = reader.getA();
//	x = reader.getX();
//	b = reader.getB();
	c = ((A) * (x));
	c = (c - b);
	for (int i = 0; i < c.getHeight(); i++) {
		sum += (c.getVal(i, 0) * c.getVal(i, 0));
	}
	sum /= c.getHeight();
	printf("Average Diff = %lf\n", sum);
	printf("Height is %d\n", c.getHeight());

	cout << "Read Complete!" << endl;
	thread.mailbox.addMessage(message);
 */
