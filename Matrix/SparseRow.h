#pragma once
#include <stdlib.h>
#include "Matrix.h"
#include "MatrixRow.h"

typedef MatrixType SparseType;

// Class to handle sparse matrix storage
// This is setup with mostly virtual functions for the purposes
// of a dynamic sparse row class that will utilize a vector
// in the future to make it easier to handle
template <typename T>
class SparseRow : public MatrixRow<T> {
public:
	SparseRow(int size) {
	    this->size = size;
	    this->values = (T*)malloc(size * sizeof(T));
	    this->index = (int*)malloc(size * sizeof(T));
	    if ((values == NULL) && (index == NULL)) {
	        this->size = 0;
	    }
	    for (int i = 0; i < this->size; i++) {
	        this->index[i] = -1;
	    }
	    this->loc = 0;
	}


	~SparseRow() {
	}

	virtual bool addVal(int ind, T value) {
    	int i = -1;
    	while ((++i < size) && (index[i] != -1));
    	if (i < size) {
    	    values[i] = value;
    	    index[i] = ind;
    	    return false;
    	} else {
    	    return true;
    	}
	}

	virtual void reset() {
		loc = 0;
	}

	virtual bool next() {
		return ((++loc < size) && (index[loc] != -1));
	}

	virtual int getIndex() {
		return index[loc];
	}

	virtual T getValue() {
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

	T getValue(int i) {
		int ind = getIndex(i);
		if (ind >= 0) {
			return values[ind];
		}
		return (T)0;
	}

	virtual int getMax() {
		int max = 0;
		for (int i = 0; i < size; i++) {
			if (index[i] > max) {
				max = index[i];
			}
		}
		return max + 1;
	}

private:
	T* values;
	int* index;
	int size;
	int loc;
};
