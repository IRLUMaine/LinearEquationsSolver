/*
 * JacobiCPU.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: jmonk
 */
#include "JacobiCPU.h"
#include <sys/time.h>
#include <iostream>
#include <sstream>

#include "../Matrix/SparseRow.h"

#define RTHRESHOLD .00001
#define THRESHOLD .00001

JacobiCPU::JacobiCPU(Mailbox** distribution, Mailbox* master, MatrixRow** rows, Matrix* b, int* rowInd, int num, int size, int id, int nProcs) {
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
    this->iterCt = new Timer("Iter");
    this->sendCt = new Timer("Send");
    this->nProcs = nProcs;

	if (!this->x) {
		perror("Problem allocating Solution Space or Send Buffer");
		this->size = 0;
		this->num = 0;
		return;
	}

	for (int i = 0; i < size; i++) {
		this->x[i] = 0;
	}
}

JacobiCPU::JacobiCPU() {
}

JacobiCPU::~JacobiCPU() {
	if (this->x) {
		delete[] this->x;
	}
    delete iterCt;
    delete sendCt;
}

/**
 * This is allows for array allocation
 */
void JacobiCPU::setControl(Mailbox** distribution, Mailbox *master, MatrixRow** rows, Matrix* b, int* rowInd, int num, int size, int id, int nProcs) {
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
    this->iterCt = new Timer("Iter");
    this->sendCt = new Timer("Send");
    this->nProcs = nProcs;



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

    struct timeval t;
    double time1, time2;
    gettimeofday(&t, NULL);
    time1 = t.tv_sec + (t.tv_usec / 1000000.0);

	while (mRun) {
		receive = false;
		iterFlag = false;
		for (int i = 0; (i < maxIter); i++) {
			convCheck = i == 0;
			iterCt->start();
			procIter();
			iterCt->stop();
		}
		sendCt->start();
		sendData();
		sendCt->stop();
		readCt = 0;
		while (mRun && readCt < (nProcs-1)) readData();
	}
    gettimeofday(&t, NULL);
    time2 = t.tv_sec + (t.tv_usec / 1000000.0);
    printf("Proc Time: %lf\n", time2 - time1);

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
		//printf("Send Iter\n");
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
//        std::stringstream ss;
		current = rowInd[i];
		SparseRow* row = dynamic_cast<SparseRow*>(rows[i]);
		row->reset();
		newVal[i] = b->getVal(current, 0);
//        ss << "b=" << newVal[i];
		do {
			j = row->getIndex();
			if (j != current) {
				newVal[i] -= row->getValue() * x[j];
//                ss << ",nv=" << newVal[i];
			}
		} while (row->next());
//        ss << ",row[cur]=" << row->getValue(current);
		newVal[i] /= row->getValue(current);
//        ss << ",nv=" << newVal[i];

		if (convCheck && !iterFlag && (fabs(x[current] - newVal[i]) > (RTHRESHOLD * fabs(x[current]) + THRESHOLD))) {
//            std::cout << ss.str() << std::endl;
//            printf("N %d %d: %g %g %g\n", i, current, fabs(x[current] - newVal[i]), x[current], newVal[i]);
			iterFlag = true;
		}
	}
	for (i = 0; i < num; i++) {
		current = rowInd[i];
//		x[current] = newVal[i];
		x[current] = (NWEIGHT * newVal[i] + LWEIGHT * x[current]) / (NWEIGHT + LWEIGHT);
	}
}


