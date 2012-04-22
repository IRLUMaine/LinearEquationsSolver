/*
 * JacobiCPU.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: jmonk
 */
#include "JacobiCPU.h"
#include "../Matrix/SparseRow.h"

#define RTHRESHOLD .001
#define THRESHOLD .0001

JacobiCPU::JacobiCPU(Mailbox** distribution, Mailbox* master, MatrixRow** rows, Matrix* b, int* rowInd, int num, int size, int id) {
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

	if (!this->x) {
		perror("Problem allocating Solution Space or Send Buffer");
		this->size = 0;
		this->num = 0;
		return;
	}

	for (int i = 0; i < size; i++) {
		this->x[i] = 1;
	}
}

JacobiCPU::JacobiCPU() {
}

JacobiCPU::~JacobiCPU() {
	if (this->x) {
		delete[] this->x;
	}
}

/**
 * This is allows for array allocation
 */
void JacobiCPU::setControl(Mailbox** distribution, Mailbox *master, MatrixRow** rows, Matrix* b, int* rowInd, int num, int size, int id) {
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


	if (!this->x) {
		perror("Problem allocating Solution Space or Send Buffer");
		this->size = 0;
		this->num = 0;
		return;
	}

	for (int i = 0; i < size; i++) {
		this->x[i] = 1;
	}
}

/**
 * @Override
 *
 * This will put the JacobiCPU into a processing
 * mode where it will run until mRun is set to false
 * using setRunning
 */
void JacobiCPU::run() {
	int c = 0;
	while (mRun) {
		receive = false;
		iterFlag = true;
		for (int i = 0; (i < maxIter) && iterFlag; i++) {
			iterFlag = false;
			procIter();
		}
		sendData();
		readCt = 0;
		while (mRun && readCt < (PROCS-1)) readData();
	}
}

/**
 * This will be called after the maximum number of iterations
 * has occured in each step. This will gather the data required
 * and send to it a networking thread to handle further distribution.
 */
void JacobiCPU::sendData() {
	if (xDistribution) {
		this->send.setType(MatrixVals);
		if (!this->sendBuf) {
			perror("Problem allocating Solution Space or Send Buffer");
			this->size = 0;
			this->num = 0;
			return;
		}
		for (int i = num; i < 2*num; i++) {
			this->sendBuf[i] = rowInd[i - num];
		}
		for (int i = 0; i < num; i++) {
			this->sendBuf[i] = x[rowInd[i]];
		}
		send.setData(sendBuf, num * 2 * sizeof(MatrixType));
		for (int i = 0; xDistribution[i] != NULL; i++) {
			xDistribution[i]->addMessage(send);
		}
		//if (!xDistributor->mailbox.isFull()) {
		//	xDistributor->mailbox.addMessage(send);
//		} else {
//			printf("Distributor Box Full %d\n", id);
		//}

		if (!receive && !master->isFull()) {
			this->send.setType(ProcStatus);

			intBuf[0] = id;
			intBuf[1] = !iterFlag;

			send.setData(intBuf, 2*sizeof(int));
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
void JacobiCPU::readData() {
	while (mailbox.hasMessage()) {
		Message message = mailbox.getMessage();

		readCt++;
		handleMessage(message);
	}
}

/**
 * This is called every time a message is received and needs to be parsed.
 */
void JacobiCPU::handleMessage(Message& message) {
	switch (message.getType()) {
	case NoMessage:
//		printf("Message: NoMessage\n");
		break;
	case StringMessage:
		printf("Received String %s\n", (char*)message.getData());
		break;
	case MatrixVals:
		receive = true;
		int length = message.getSize() / sizeof(MatrixType) / 2;
		const MatrixType* data = (const MatrixType*)message.getData();
		for (int i = 0; i < length; i++) {
			x[(int)data[length + i]] = data[i];
		}
		break;
	}
}

#define NWEIGHT 1
#define LWEIGHT 4

/**
 * This will be called for each iteration to be processed and will
 * update x with the new values.
 */
void JacobiCPU::procIter() {
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

		if (fabs(x[current] - newVal[i]) > (RTHRESHOLD * fabs(x[i]) + THRESHOLD)) {
			iterFlag = true;
		}
	}
	for (i = 0; i < num; i++) {
		current = rowInd[i];
//		x[current] = newVal[i];
		x[current] = (NWEIGHT * newVal[i] + LWEIGHT * x[current]) / (NWEIGHT + LWEIGHT);
	}
}


