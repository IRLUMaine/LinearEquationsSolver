#pragma once
#include <stdlib.h>
#include "Matrix.h"
#include "MatrixRow.h"

/**
 * Class to handle sparse matrix storage
 * MatrixTypehis is setup with mostly virtual functions for the purposes
 * of a dynamic sparse row class that will utilize a vector
 * in the future to make it easier to handle
 */
class SparseRow : public MatrixRow {
public:
	/**
	 * Creates an empty sparse row. It can only hold a maximum of size elements.
	 */
	SparseRow(int size) {
	    this->size = size;
	    this->values = (MatrixType*)malloc(size * sizeof(MatrixType));
	    this->index = (int*)malloc(size * sizeof(MatrixType));
	    if ((values == NULL) && (index == NULL)) {
	        this->size = 0;
	    }
	    for (int i = 0; i < this->size; i++) {
	        this->index[i] = -1;
	    }
	    this->loc = 0;
	}

	SparseRow(const SparseRow& other) {
	    this->size = other.size;
	    this->values = (MatrixType*)malloc(size * sizeof(MatrixType));
	    this->index = (int*)malloc(size * sizeof(MatrixType));
	    for (int i = 0; i < this->size; i++) {
	        this->index[i] = other.index[i];
	        this->values[i] = other.values[i];
	    }
	    this->loc = 0;
	}

	virtual ~SparseRow() {
		if (size > 0) {
			free(this->values);
			free(this->index);
		}
	}

	MatrixType getValue(int i) {
		int ind = getIndex(i);
		if (ind >= 0) {
			return values[ind];
		}
		return (MatrixType)0;
	}

	/**
	 * MatrixTypehis inserts a value at the column ind, in the first available space.
	 */
	virtual bool addVal(int ind, MatrixType value) {
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

	/**
	 * MatrixTypehe following is a set of functions to facilitate sparse access. MatrixTypehe life
	 * -cycle of said access would be something like as follows.
	 *
	 * row.reset();
	 * do {
	 * 		int index = row.getIndex();
	 * 		MatrixType val = row.getValue();
	 * 		// Do stuff
	 * } while (row.next());
	 *
	 * Reset will start the getIndex(), getValue(), and next() functions to the
	 * beginning of the row.
	 */
	virtual void reset() {
		loc = 0;
	}

	/**
	 * Moves the row ahead by elements. Only calling this or reset() can change
	 * the values returned by getIndex() and getValue().
	 *
	 * next() will return true if there is a new value to be received by
	 * getValue(). Otherwise the end of row has been reached and it will return
	 * false.
	 */
	virtual bool next() {
		return ((++loc < size) && (index[loc] != -1));
	}

	/**
	 * Returns what column the current element is located in.
	 */
	virtual int getIndex() {
		return index[loc];
	}

	/**
	 * Returns the value of the current element.
	 */
	virtual MatrixType getValue() {
		return values[loc];
	}

	/**
	 * Gets index of the ith element stored in the sparse row.
	 */
	virtual int getIndex(int i) {
		for (int j = 0;j < size; j++) {
			if (index[j] == i) {
				return j;
			}
		}
		return -1;
	}

	/**
	 * Gets the highest column index in this row.
	 */
	virtual int getMax() {
		int max = 0;
		for (int i = 0; i < size; i++) {
			if (index[i] > max) {
				max = index[i];
			}
		}
		return max + 1;
	}

	/**
	 * This does a deep copy of sparse row.
	 */
	SparseRow& operator=(SparseRow& other) {
	    this->size = other.size;
	    this->values = (MatrixType*)malloc(size * sizeof(MatrixType));
	    this->index = (int*)malloc(size * sizeof(MatrixType));
	    for (int i = 0; i < this->size; i++) {
	        this->index[i] = other.index[i];
	        this->values[i] = other.values[i];
	    }
	    this->loc = 0;
	    return other;
	}

	MatrixType* getValues() {
		return values;
	}

	int* getIndexs() {
		return index;
	}

private:
	MatrixType* values;
	int* index;
	int size;
	int loc;
};
