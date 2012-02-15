#include "SparseRow.h"

SparseRow::SparseRow(int size) {
	this->size = size;
	this->values = (SparseType*)malloc(size * sizeof(SparseType));
	this->index = (int*)malloc(size * sizeof(SparseType));
	if ((values == NULL) && (index == NULL)) {
		this->size = 0;
	}
	for (int i = 0; i < this->size; i++) {
		this->index[i] = -1;
	}
	this->loc = 0;
}

bool SparseRow::addVal(int ind, SparseType value) {
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

