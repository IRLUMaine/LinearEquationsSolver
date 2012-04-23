/*
 * ResidualCalculator.h
 *
 *  Created on: Apr 22, 2012
 *      Author: jmonk
 */
#ifndef RESIDUAL_CALCULATOR_H_
#define RESIDUAL_CALCULATOR_H_
#include "../Communication/Thread.h"
#include "../Matrix/SparseMatrix.h"

class ResidualCalculator : public Thread {
public:
	ResidualCalculator(Matrix* A, Matrix *b, Matrix *x) {
		this->A = (SparseMatrix*)A;
		this->b = b;
		this->x = x;
		mRun = true;
	}
	~ResidualCalculator() {

	}

	/**
	 * This sits in the same comm network as the processors. It will
	 * collect messages and as x values are updated it will calculate
	 * the new residual and output it.
	 */
	void run();

	bool mRun;
private:
	SparseMatrix *A;
	Matrix *b;
	Matrix *x;
	Matrix result;
};

#endif /* DISTRIBUTOR_H_ */
