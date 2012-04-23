/*
 * ResidualCalculator.cpp
 *
 *  Created on: Apr 22, 2012
 *      Author: jmonk
 */

#include "ResidualCalculator.h"
#include <math.h>

void ResidualCalculator::run() {
	while (mRun) {
		while (mailbox.hasMessage()) {
            Message message = mailbox.getMessage();
            switch (message.getType()) {
            case MatrixVals:
		        int length = message.getSize() / sizeof(MatrixType) / 2;
		        const MatrixType* data = (const MatrixType*)message.getData();
		        for (int i = 0; i < length; i++) {
		            x->setVal((int)data[length + i], 0, data[i]);
		        }
				break;
			}
		}
		result = (*A * *x) - *b;
		MatrixType res = 0;
		MatrixType max = 0;
		MatrixType temp = 0;
		int ind = -1;
		for (int i = 0; i < result.getHeight(); i++) {
			res += (result[i][0] * result[i][0]);
			temp = fabs(result[i][0]);
			if (temp > max) {
				max = temp;
				ind = i;
			}
		}
		res = sqrt(res);
		printf("Residual is %lf Max: %lf %d\n%lf %lf/%lf=%lf\n", res, max, ind, (*x)[ind][0], (*b)[ind][0], A->getVal(ind, ind), (*b)[ind][0]/A->getVal(ind, ind));
	}
}



