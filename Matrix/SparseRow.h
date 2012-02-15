#pragma once
#include <stdlib.h>
#include "Matrix.h"

typedef MatrixType SparseType;

// Class to handle sparse matrix storage
// This is setup with mostly virtual functions for the purposes
// of a dynamic sparse row class that will utilize a vector
// in the future to make it easier to handle
class SparseRow {
public:
	SparseRow(int size);

	~SparseRow() {
	}

	virtual bool addVal(int i, SparseType value);

	virtual void reset() {
		loc = 0;
	}

	virtual bool next() {
		return ((++loc < size) && (index[loc] != -1));
	}

	virtual int getIndex() {
		return index[loc];
	}

	virtual SparseType getValue() {
		return values[loc];
	}

	virtual int getIndex(int i) {
		for (int j = 0;j < size; j++) {
			if (index[j] == i) {
				return j;
			}
		}
		return -1;
	}

	virtual SparseType getValue(int i) {
		int ind = getIndex(i);
		if (ind >= 0) {
			return values[ind];
		}
		return (SparseType)0;
	}

	int getMax() {
		int max = 0;
		for (int i = 0; i < size; i++) {
			if (index[i] > max) {
				max = index[i];
			}
		}
		return max + 1;
	}

private:
	SparseType* values;
	int* index;
	int size;
	int loc;
};
