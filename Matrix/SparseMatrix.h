#pragma once
#include "SparseRow.h"
#include "Matrix.h"
#include <iostream>

using namespace std;

// Mostly currently a smart holder for SparseRows
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

	void setRow(int index, SparseRow* row) {
		if (index < nrows) {
			rows[index] = row;
		}
	}

	SparseRow &getRow(int index) {
		if (index < nrows) {
			return *rows[index];
		}
		return *(new SparseRow(10));
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
			int nmax = getRow(i).getMax();
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
				SparseRow row = getRow(i);
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
	SparseRow** rows;
	int *count;
	int nrows;
};
