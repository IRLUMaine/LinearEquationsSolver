/*
 * JacobiGPU.cpp
 *
 *  Created on: Apr 9, 2012
 *      Author: jmonk
 */
#include "JacobiGPU.h"
#include "JacobiCPU.h"
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <sys/time.h>
#include "../Matrix/SparseRow.h"

void computeIterations(int id, int varPBlock, int maxRow, int num, int size,
		int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int* dDiffs);
void rearrangeCall(int id, int varPBlock, int maxRow, int num, int size,
		int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int* dDiffs);

JacobiGPU::JacobiGPU(Mailbox** distribution, Mailbox* master, MatrixRow** rows,
		Matrix* b, int* rowInd, int num, int size, int id) {
	setControl(distribution, master, rows, b, rowInd, num, size, id);
}

JacobiGPU::JacobiGPU() {
}

JacobiGPU::~JacobiGPU() {
	if (this->x) {
		delete[] this->x;
	}
	delete iterCt;
	delete sendCt;
}

/**
 * This is allows for array allocation
 */
void JacobiGPU::setControl(Mailbox** distribution, Mailbox *master,
		MatrixRow** rows, Matrix* b, int* rowInd, int num, int size, int id) {
	this->rows = rows;
	this->size = size;
	this->rowInd = rowInd;
	this->num = num;
	this->b = b;
	this->xDistribution = distribution;
	this->master = master;
	this->mRun = true;
	this->maxIter = 1;
	this->x = new MatrixType[size];
	this->id = id;
	this->sendBuf = new MatrixType[num * 2];
	this->intBuf = new int[2];
	this->varPBlock = 1024;
	this->iterCt = new Timer("Iter");
	this->sendCt = new Timer("Send");
}

void JacobiGPU::setup() {
	Timer setup("Setup");
	setup.start();

	mailbox.setLocking(false);
	if (!this->x) {
		perror("Problem allocating Solution Space or Send Buffer");
		this->size = 0;
		this->num = 0;
		return;
	}
	cudaError_t error;
	int count = -1;
	cudaSafe(cudaGetDeviceCount(&count), "Get Device Count");
	printf("Found %d Devices\n", count);
	cudaSafe(cudaSetDevice(id), "Set Device");

	for (int i = 0; i < size; i++) {
		this->x[i] = 1;
	}

	// Following section needs some error checking
	// Also some row fixing
	maxRow = 0;
	for (int i = 0; i < num; i++) {
		int max = ((SparseRow*) rows[i])->getCnt();
		if (max > maxRow) {
			maxRow = max;
		}
	}
	maxRow++;
	cudaSafe(cudaMalloc((void **)&dXs, sizeof(MatrixType) * size), "Malloc Xs");
	//cudaSafe(cudaMalloc((void **)&dNextXs, sizeof(MatrixType) * size), "Malloc NXs");
	cudaSafe(cudaMemset((void *)dXs, 0, sizeof(MatrixType) * size),
			"Memset Xs");

	cudaSafe(cudaMalloc((void **)&dIndexs, sizeof(int) * num),
			"Malloc Indexes");

	printf("Rows take: %d\n", sizeof(MatrixType) * num * maxRow);
	cudaSafe(cudaMalloc((void **)&dRowVals, sizeof(MatrixType) * num * maxRow),
			"Malloc RowVals");
	cudaSafe(cudaMalloc((void **)&dRowInds, sizeof(int) * num * maxRow),
			"Malloc RowInds");
	printf("Allocated dIndexs %p\n", dIndexs);
	cudaSafe(
			cudaMalloc((void **)&dRowVRearr, sizeof(MatrixType) * num * maxRow),
			"Malloc RowVRearr");
	cudaSafe(cudaMalloc((void **)&dRowIRearr, sizeof(int) * num * maxRow),
			"Malloc RowIRearr");
	cudaSafe(cudaMalloc((void **)&dYs, sizeof(MatrixType) * num), "Malloc Ys");
	cudaSafe(cudaMalloc((void **)&dDiffs, sizeof(MatrixType) * (num/1024 + 1)),
			"Malloc Diffs");
	cudaSafe(
			cudaMallocHost((void **)&hDiffs, sizeof(MatrixType) * (num/1024 + 1)),
			"Malloc Host Diffs");

	printf("%d\n", rowInd[0]);
	cudaSafe(
			cudaMemcpy(dYs, &b->getRaw()[rowInd[0]], sizeof(MatrixType) * num, cudaMemcpyHostToDevice),
			"Memcpy Ys");

	cudaSafe(
			cudaMemcpy(dIndexs, rowInd, sizeof(int) * num, cudaMemcpyHostToDevice),
			"Memcpy Indexes");
	struct timeval t;
	double time1, time2;

	gettimeofday(&t, NULL);
	time1 = t.tv_sec + (t.tv_usec / 1000000.0);

	printf("MaxRow: %d - Num: %d\n", maxRow, num * id);
	cudaSafe(
			cudaMemcpy(dRowVals, ((SparseRow*)rows[0])->getValues(), sizeof(MatrixType) * maxRow * num, cudaMemcpyHostToDevice),
			"Memcpy RowVals");
	cudaSafe(
			cudaMemcpy(dRowInds, ((SparseRow*)rows[0])->getIndexs(), sizeof(int) * maxRow * num, cudaMemcpyHostToDevice),
			"Memcpy RowInds");
	/*
	 for (int i = 0; i < num; i++) {
	 SparseRow *row = (SparseRow*)rows[i];
	 cudaSafe(cudaMemcpy(dRowVals + (i * maxRow), row->getValues(), sizeof(MatrixType) * maxRow, cudaMemcpyHostToDevice), "Memcpy RowVals");
	 cudaSafe(cudaMemcpy(dRowInds + (i * maxRow), row->getIndexs(), sizeof(int) * maxRow, cudaMemcpyHostToDevice), "Memcpy RowInds");
	 }
	 */

	for (int i = num; i < 2 * num; i++) {
		this->sendBuf[i] = rowInd[i - num];
	}

	setup.stop();
}

/**
 * @Override
 *
 * This will put the JacobiGPU into a processing
 * mode where it will run until mRun is set to false
 * using setRunning
 */
void JacobiGPU::run() {
	int c = 0;
	setup();
	Timer proctimer("Processing");
	proctimer.start();
	cudaError_t error;
	cudaSafe(cudaSetDevice(id), "Set Device");

	rearrangeCall(id, varPBlock, maxRow, num, size, maxIter, dXs, dNextXs,
			dIndexs, dRowVals, dRowVRearr, dRowInds, dRowIRearr, dYs, dDiffs);

	while (mRun) {
		receive = false;
		//iterFlag = true;
		iterCt->start();
		//printf("Iteration: %d %d\n", id, num);
		computeIterations(id, varPBlock, maxRow, num, size, maxIter, dXs,
				dNextXs, dIndexs, dRowVals, dRowVRearr, dRowInds, dRowIRearr,
				dYs, dDiffs);
		iterCt->stop();
		//for (int i = 0; (i < maxIter) && iterFlag; i++) {
		//	iterFlag = false;
		//	procIter();
		//}
		sendCt->start();
		sendData();
		sendCt->stop();
		readCt = 0;
		while (mRun && readCt < (PROCS - 1))
			readData();
	}
	cudaSafe(cudaMemcpy(x+rowInd[0], dXs+rowInd[0], sizeof(MatrixType) * num, cudaMemcpyDeviceToHost),
			"Memcpy X Vals Down");

	proctimer.stop();
	proctimer.display();
}

/**
 * This will be called after the maximum number of iterations
 * has occured in each step. This will gather the data required
 * and send to it a networking thread to handle further distribution.
 */
void JacobiGPU::sendData() {
	if (xDistribution) {
		cudaError_t error;
		cudaSafe(
				cudaMemcpy(sendBuf, dXs+rowInd[0], sizeof(MatrixType) * num, cudaMemcpyDeviceToHost),
				"Memcpy X Vals Down");
		this->send.setType(MatrixVals);
		if (!this->sendBuf) {
			perror("Problem allocating Solution Space or Send Buffer");
			this->size = 0;
			this->num = 0;
			return;
		}
//		for (int i = 0; i < num; i++) {
//			this->sendBuf[i] = x[rowInd[i]];
//		}

		send.setData(sendBuf, num * 2 * sizeof(MatrixType));
		for (int i = 0; xDistribution[i] != NULL; i++) {
			xDistribution[i]->addMessage(send);
		}
		//if (!xDistributor->mailbox.isFull()) {
		//	xDistributor->mailbox.addMessage(send);
//		} else {
//			printf("Distributor Box Full %d\n", id);
		//}

		cudaSafe(cudaMemcpy(hDiffs, dDiffs, sizeof(MatrixType) * (num/1024 + 1), cudaMemcpyDeviceToHost),
				"Memcpy Diffs");
		int size = num / 1024;
		if (1024 * size != num) {
			size++;
		}
		iterFlag = 0;
		//MatrixType max = 0;
		for (int i = 0; i < num; i++) {
			if (hDiffs[i] > 0) {
				iterFlag = true;
				break;
			}
			//if (hDiffs[i] > max) {
			//	max = hDiffs[i];
			//}
		}
		//printf("Max is %g\n", max);
		if (!receive && !master->isFull()) {
			this->send.setType(ProcStatus);

			intBuf[0] = id;
			intBuf[1] = !iterFlag;

			send.setData(intBuf, 2 * sizeof(int));
			master->addMessage(send);
//		} else {
//			printf("Distributor Box Full %d\n", id);
		}
	}
}

/**
 * This will read in all messages from the mailbox and handle them
 * accordingly. This can include changes to config as well as updated
 * x information.
 */
void JacobiGPU::readData() {
	while (mailbox.hasMessage()) {
		Message message = mailbox.getMessage();

		readCt++;
		handleMessage(message);
	}
}

/**
 * This is called every time a message is received and needs to be parsed.
 */
void JacobiGPU::handleMessage(Message& message) {
	switch (message.getType()) {
	case NoMessage:
//		printf("Message: NoMessage\n");
		break;
	case StringMessage:
		printf("Received String %s\n", (char*) message.getData());
		break;
	case MatrixVals:
		receive = true;
		int length = message.getSize() / sizeof(MatrixType) / 2;
		const MatrixType* data = (const MatrixType*) message.getData();

		cudaError_t error;
		cudaSafe(
				cudaMemcpy(dXs+((int)data[length]), data, sizeof(MatrixType) * length, cudaMemcpyHostToDevice),
				"Memcpy Recv Vals");
//		for (int i = 0; i < length; i++) {
//			x[(int) data[length + i]] = data[i];
//		}
		break;
	}
}

#define NWEIGHT 1
#define LWEIGHT 4

/**
 * This will be called for each iteration to be processed and will
 * update x with the new values.
 */
void JacobiGPU::procIter() {
	int current;
	int i, j;
	MatrixType newVal[num];
	for (i = 0; i < num; i++) {
		current = rowInd[i];
		SparseRow* row = dynamic_cast<SparseRow*>(rows[i]);
		row->reset();
		newVal[i] = b->getVal(current, 0);
		do {
			j = row->getIndex();
			if (j != current) {
				newVal[i] -= row->getValue() * x[j];
			}
		} while (row->next());
		newVal[i] /= row->getValue(current);

		//if (fabs(x[current] - newVal[i]) > (RTHRESHOLD * fabs(x[i]) + THRESHOLD)) {
		//	iterFlag = true;
		//}
	}
	for (i = 0; i < num; i++) {
		current = rowInd[i];
//		x[current] = newVal[i];
		x[current] = (NWEIGHT * newVal[i] + LWEIGHT * x[current])
				/ (NWEIGHT + LWEIGHT);
	}
}

