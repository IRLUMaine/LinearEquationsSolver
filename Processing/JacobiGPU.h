#pragma once
#include "../Communication/Thread.h"
#include "../Matrix/Matrix.h"
#include "../Util/Timer.h"

#define SYNC

#define cudaSafe(x, y) if ((error = x) != cudaSuccess) {\
	printf("%s Error: %s\n", y, cudaGetErrorString(error));\
	exit(1);\
	}

class JacobiGPU : public Thread {
public:
	JacobiGPU(Mailbox** distribution, Mailbox* master, MatrixRow** rows, Matrix *b, int* rowInd, int num, int size, int id, int nProcs);
	JacobiGPU();

	~JacobiGPU();

	/**
	 * This is allows for array allocation
	 */
	void setControl(Mailbox** distributor, Mailbox* master, MatrixRow** rows, Matrix *b, int* rowInd, int num, int size, int id, int nProcs);

	/**
	 * @Override
	 *
	 * This will put the JacobiGPU into a processing
	 * mode where it will run until mRun is set to false
	 * using setRunning
	 */
	void run();

	/**
	 * This controls whether the processing thread will
	 * continue running. If set to false the thread will
	 * stop running after the current set of iterations is
	 * complete. The length of a set of iterations can be
	 * controlled using setMaxIter().
	 */
	void setRunning(bool running) {
		mRun = running;
	}

	/**
	 * Checks to see if the processor is going to continue
	 * processing iterations. Returning false does not
	 * gaurantee that the thread is not running, simply that
	 * after the current step it will not be running.
	 */
	bool isRunning() {
		return mRun;
	}

	/**
	 * This controls the maximum number of iterations that
	 * can occur between a communication step.
	 */
	void setMaxIter(int max) {
		maxIter = max;
	}

	/**
	 * Gets the maximum number of iterations that can occur
	 * between communication steps.
	 */
	int getMaxIter() {
		return maxIter;
	}

	MatrixType* getX() {
		return x;
	}

	int getSize() {
		return num;
	}

protected:
    int nProcs;

private:
	/**
	 * This will be called after the maximum number of iterations
	 * has occured in each step. This will gather the data required
	 * and send to it a networking thread to handle further distribution.
	 */
	void sendData();

	/**
	 * This will read in all messages from the mailbox and handle them
	 * accordingly. This can include changes to config as well as updated
	 * x information.
	 */
	void readData();

	/**
	 * This is called every time a message is received and needs to be parsed.
	 */
	void handleMessage(Message& message);

	/**
	 * This will be called for each iteration to be processed and will
	 * update x with the new values.
	 */
	void procIter();

	void setup();

	int num;
	bool mRun;
	int maxIter;
	bool receive;
	int id;
	int readCt;
	MatrixType *x;
	Matrix *b;
	MatrixType *sendBuf;
	int* intBuf;
	MatrixRow **rows;
	Mailbox **xDistribution;
	Mailbox *master;
	int *rowInd;
	int size;
	bool iterFlag;
	Message send;

	// Added for CUDA Support
	int varPBlock;
	int maxRow;
	MatrixType *dXs;
	MatrixType *dNextXs;
	int *dIndexs;
	MatrixType *dRowVals;
	int *dRowInds;
	MatrixType *dRowVRearr;
	int *dRowIRearr;
	MatrixType *dYs;
	int *dDiffs;
	int *hDiffs;

	// Added for timing support
	Timer* iterCt;
	Timer* sendCt;
};
