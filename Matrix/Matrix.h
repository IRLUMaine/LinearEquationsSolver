#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "MatrixRow.h"

typedef float MatrixType;

// Basic matrix class to be used in conjunction
// with the SparseMatrix class, will be instantiated
// upon any sort of operation (+,-,*), regardless
// of subtypes being Sparse or not.

// FEATURE This class could use some more work
class Matrix {
public:
	Matrix(int nrow, int ncol) {
		this->nrow = nrow;
		this->ncol = ncol;
		matrix = (MatrixType*) malloc(ncol * nrow * sizeof(MatrixType) + 1);
		if (matrix == NULL) {
			this->nrow = 0;
			this->ncol = 0;
			fprintf(stderr, "Error: Matrix could not allocate %d bytes\n", ncol*nrow*sizeof(MatrixType));
		} else {
			matrix[0] = 1;
			matrix++;
		}
	}

	Matrix(const Matrix& other) {
		this->nrow = other.nrow;
		this->ncol = other.ncol;
		this->matrix = other.matrix;
		matrix[-1]++;
	}

	Matrix() {
		// For Subclasses no allocation needed
		matrix = NULL;
		nrow = 0;
		ncol = 0;
	}

	~Matrix() {
		if (matrix != NULL) {
			matrix--;
			matrix[0]--;
			if (matrix[0] == 0) {
				free(matrix);
			}
		}
	}

	virtual MatrixType getVal(int ir, int ic) {
		if ((ir >= nrow) || (ic >= ncol)) {
			fprintf(stderr, "Error: Attempted to read outside Matrix bounds %d %d %d %d\n", ir, ic, nrow, ncol);
			return 0;
		}
		return matrix[ncol * ir + ic];
	}

	virtual void setVal(int ir, int ic, MatrixType val) {
		if ((ir >= nrow) || (ic >= ncol)) {
			fprintf(stderr, "Error: Attempted to write outside Matrix bounds %d %d %d %d\n", ir, ic, nrow, ncol);
			return;
		}
		matrix[ncol * ir + ic] = val;
	}

	virtual int getWidth() {
		return ncol;
	}

	virtual int getHeight() {
		return nrow;
	}

	Matrix operator+(Matrix &other) {
		int ncol = getWidth();
		int nrow = getHeight();
		Matrix matrix(nrow, ncol);

		for (int i = 0; i < nrow; i++) {
			for (int j = 0; j < ncol; j++) {
				matrix.setVal(i, j, getVal(i, j) + other.getVal(i, j));
			}
		}

		return matrix;
	}

	Matrix operator-(Matrix &other) {
		int ncol = getWidth();
		int nrow = getHeight();
		Matrix matrix(nrow, ncol);

		for (int i = 0; i < nrow; i++) {
			for (int j = 0; j < ncol; j++) {
				matrix.setVal(i, j, getVal(i, j) - other.getVal(i, j));
			}
		}

		return matrix;
	}

	Matrix operator/(Matrix &other) {
		int ncol = getWidth();
		int nrow = getHeight();
		Matrix matrix(nrow, ncol);

		for (int i = 0; i < nrow; i++) {
			for (int j = 0; j < ncol; j++) {
				matrix.setVal(i, j, getVal(i, j) / other.getVal(i, j));
			}
		}

		return matrix;
	}

	Matrix operator*(Matrix &other)  {
		int ncol = getWidth();
		int nrow = getHeight();
		int otherWidth = other.getWidth();
		double percent = 0;
		Matrix matrix(nrow, otherWidth);

		for (int i = 0; i < nrow; i++) {
			for (int j = 0; j < otherWidth; j++) {
				MatrixType sum = 0;
				for (int k = 0; k < ncol; k++) {
					sum += getVal(i, k) * other.getVal(k, j);
				}
				matrix.setVal(i, j, sum);
			}
		}

		return matrix;
	}

	Matrix &operator=(const Matrix &other)  {
		matrix[-1]--;
		if (matrix[-1] == 0) {
			free(--matrix);
		}

		this->nrow = other.nrow;
		this->ncol = other.ncol;
		this->matrix = other.matrix;
		matrix[-1]++;

		return *this;
	}

	virtual MatrixRow<MatrixType> getRow(int i) {
		if (i < nrow) {
			MatrixRow<MatrixType> matrixRow(&matrix[i*ncol], ncol);
			return matrixRow;
		} else {
			MatrixRow<MatrixType> matrixRow;
			return matrixRow;
		}
	}

private:
	MatrixType* matrix;
	int nrow;
	int ncol;
};

