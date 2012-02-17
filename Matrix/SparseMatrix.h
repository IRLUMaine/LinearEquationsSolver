#pragma once
#include "SparseRow.h"
#include "Matrix.h"
#include <iostream>

using namespace std;

// Mostly currently a smart holder for SparseRow<SparseType>s
// As time goes on will develop to have some code
// to make sparse handling easier
class SparseMatrix : public Matrix {
public:
	// This is used to limit all of the rows to the same max size
	SparseMatrix(int nrows, int ncolmax);

	// When this constructor is used all of the rows must be set
	// using the setRow function.
	SparseMatrix(int nrows);

	SparseMatrix(const SparseMatrix& other);

	~SparseMatrix();

	void setRow(int index, SparseRow<SparseType>* row) {
		if (index < nrows) {
			rows[index] = row;
		}
	}

	MatrixRow<SparseType> getRow(int index) {
		if (index < nrows) {
			return *rows[index];
		}
		SparseRow<SparseType> sparseRow(10);
		return sparseRow;
	}

	virtual SparseRow<SparseType> getRowS(int index) {
		if (index < nrows) {
			return *rows[index];
		}
		SparseRow<SparseType> sparseRow(10);
		return sparseRow;
	}

	MatrixType getVal(int ir, int ic) {
		return getRow(ir).getValue(ic);
	}

	int getHeight() {
		return nrows;
	}

	int getWidth() {
		int max = 0;
		for (int i = 0; i < nrows; i++) {
			int nmax = getRowS(i).getMax();
			if (nmax > max) {
				max = nmax;
			}
		}
		return max;
	}

	void setVal(int ir, int ic, MatrixType val) {
		cout << "Warning: setVal called on SparseMatrix" << endl;
	}

	SparseMatrix &operator=(SparseMatrix& other);

	Matrix operator*(Matrix& other) {
        int nrow = getWidth();
        int ncol = getHeight();
        int otherWidth = other.getWidth();
        double percent = 0;
        Matrix matrix(nrow, otherWidth);

        for (int i = 0; i < nrow; i++) {
            if (((double)i) / nrow > percent) {
                printf("%lf\n", 100*percent);
                percent += .001;
            }
            for (int j = 0; j < otherWidth; j++) {
                MatrixType sum = 0;
				SparseRow<SparseType> row = getRowS(i);
				row.reset();
				do {
					int index = row.getIndex();
					SparseType val = row.getValue();
					sum += val * other.getVal(index, j);
				} while (row.next());
                matrix.setVal(i, j, sum);
            }
        }

        return matrix;

	}

private:
	SparseRow<SparseType>** rows;
	int *count;
	int nrows;
};
