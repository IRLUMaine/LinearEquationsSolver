#pragma once
#include "Matrix.h"

/**
 * Basic class used to double index arrays as well as allow for subclasses like
 * SparseRow.
 */
class MatrixRow {
public:
	MatrixRow(MatrixType* row, int size) {
		this->row = row;
		this->size = size;
	}

	MatrixRow() {
		// For subclasses
		size = 0;
		row = NULL;
	}

	virtual ~MatrixRow() {
	}

	virtual MatrixType getValue(int i) {
		if (i < size) {
			return row[i];
		}
		printf("Error: %d is not less than %d\n", i, size);
		return -1;
	}

	MatrixType operator[](int index) {
		return row[index];
	}
private:
	MatrixType *row;
	int size;
};
