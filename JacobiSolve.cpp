#include <iostream>
#include "Matrix/Matrix.h"
#include "Matrix/FastookReader.h"
#include "Communication/Thread.h"
#include "Communication/Message.h"
#include "Processing/JacobiCPU.h"
#include "Processing/JacobiGPU.h"
#include "Processing/Distributor.h"
#include "Processing/ResidualCalculator.h"
#include "Util/Timer.h"

#define Solver JacobiGPU

using namespace std;
#define COMPLETE_THRESHOLD 1
//5

#define ROW 301
#define COL 561

int main(int argc, char * argv[])
{
	Timer setup("Read");
	Timer comp("Computation");
	setup.start();
	// Produce some classes
	FastookReader reader("matrix.file");
	Solver jacobiSolver[PROCS];
	//Distributor distributor[PROCS];
	Mailbox collection[PROCS];
	Mailbox *list[PROCS][PROCS+1];
	FILE *file;
	int completeFlags[PROCS] = {0};
	bool done = false;

	if (argc > 1) {
		file = fopen(argv[1], "w");
	} else {
		file = fopen("output", "w");
	}

	// Setup which distributor controls which thread
	// When PROC=1 these do essentially nothing
	for (int i = 0; i < PROCS; i++) {
		collection[i].setLocking(false);
		//distributor[i].setControl(list[i], &jacobiSolver[i], &collection);
	}

	// Matrix setup stuff
	reader.readFile();
	MatrixRow** rows[PROCS];
	int* rowInd[PROCS];
	int height = reader.getA()->getHeight();
	int size = height/PROCS;
	ResidualCalculator res(reader.getA(), reader.getB(), reader.getX());

	// Allocate memory for row pointer storage
	// Setup distribution lists
	for (int i = 0; i < PROCS; i++) {
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
		for (int j = 0; j < i; j++) {
			list[i][j] = &jacobiSolver[j].mailbox;
		}
		for (int j = i+1; j < PROCS; j++) {
			list[i][j-1] = &jacobiSolver[j].mailbox;
		}
		//list[i][PROCS-1] = &res.mailbox;
		list[i][PROCS-1] = NULL;
	}

	// Fill in row pointer memory
	// This distributes what PROCS solve which vars
	for (int j = 0; j < PROCS; j++) {
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

	// Finally give setup control data to jacobi PROCS
	// Also setup max number of iterations between comm
	// packets.
	for (int i = 0; i < PROCS; i++) {
		int j;
		for (j = 0; j < size; j++) {
			if (rows[i][j] == NULL) {
				break;
			}
		}
		jacobiSolver[i].setControl(list[i], collection+i, rows[i], reader.getB(), rowInd[i], j, height, i);
		jacobiSolver[i].setMaxIter(14);
	}

	// Start all threads
	for (int i = 0; i < PROCS; i++) {
		//distributor[i].start();
		jacobiSolver[i].start();
	}
	//res.start();
	// Wait till everyone done.
	while (!done) {
		// Check for status messages
		for (int i = 0; i < PROCS; i++) {
			while (collection[i].hasMessage()) {
				Message message = collection[i].getMessage();
				if (message.getType() == ProcStatus) {
					int* buf = (int *)message.getData();
					if (buf[1]) {
						// Wait a little bit after completion to make sure all are really done
						// counting to some number solves this
						completeFlags[buf[0]]++;
					} else {
						completeFlags[buf[0]] = 0;
					}
					done = true;
					for (int i = 0; i < PROCS; i++) {
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
	for (int i = 0; i < PROCS; i++) {
		jacobiSolver[i].setRunning(false);
		jacobiSolver[i].stop();
	}
	res.mRun = false;
	comp.stop();
	//res.stop();
	// Print some results.
	for (int j = 0; j < PROCS; j++) {
		for (int i = 0; i < size; i++) {
			fprintf(file, "%lf\n", jacobiSolver[j].getX()[i + j * (size)]);
		}
	}
//	for (int i = 0; i < PROCS; i++) {
//		if (jacobiSolver[i].isAwake()) {
//			jacobiSolver[i].stop();
//		}
//		if (distributor[i].isAwake()) {
//			distributor[i].stop();
//		}
//	}
	// Free some memory
	for (int i = 0; i < PROCS; i++) {
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
	b.setVal(PROCS, 0, 4PROCS);

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
