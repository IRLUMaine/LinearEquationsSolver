#include "SparseMatrix.h"


SparseMatrix::SparseMatrix(int nrows, int ncolmax) {
	this->nrows = nrows;
	this->rows = (SparseRow**)malloc(nrows * sizeof(SparseRow*));
	if (rows == NULL) {
		this->nrows = 0;
	}

	for (int i = 0; i < nrows; i++) {
		this->rows[i] = new SparseRow(ncolmax);
	}
	this->count = (int*)malloc(sizeof(int));

	// Too lazy for ton of checks
	// if null pointer exception on
	// this line then allocation failed
	this->count[0] = 1.0;
}

SparseMatrix::SparseMatrix(int nrows) {
	this->nrows = nrows;
	this->rows = (SparseRow**)malloc(nrows * sizeof(SparseRow*));
	if (rows == NULL) {
		this->nrows = 0;
	}

	for (int i = 0; i < nrows; i++) {
		this->rows[i] = NULL;
	}
	this->count = (int*)malloc(sizeof(int));

	// Too lazy for ton of checks
	// if null pointer exception on
	// this line then allocation failed
	this->count[0] = 1.0;
}

SparseMatrix::SparseMatrix(const SparseMatrix& other) {
	this->count[0]--;
	if (this->count[0] == 0) {
		for (int i = 0; i < nrows; i++) {
			if (rows[i] != NULL) {
				delete rows[i];
			}
		}
		free(rows);
	}

	this->count = other.count;
	this->count++;
	this->nrows = other.nrows;
	this->rows = other.rows;
}

SparseMatrix &SparseMatrix::operator=(SparseMatrix& other) {
    this->count[0]--;
    if (this->count[0] == 0) {
        for (int i = 0; i < nrows; i++) {
            if (rows[i] != NULL) {
                delete rows[i];
            }
        }
        free(rows);
    }

    this->count = other.count;
    this->count++;
    this->nrows = other.nrows;
    this->rows = other.rows;
}

SparseMatrix::~SparseMatrix() {
	this->count[0]--;
	if (this->count[0] == 0) {
		for (int i = 0; i < nrows; i++) {
			if (rows[i] != NULL) {
				delete rows[i];
			}
		}
		free(rows);
	}
}
