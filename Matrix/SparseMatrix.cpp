#include "SparseMatrix.h"


SparseMatrix::SparseMatrix(int nrows, int ncolmax) {
	this->nrows = nrows;
	this->rows = (SparseRow**)malloc(nrows * sizeof(SparseRow*));
	this->values = (MatrixType*)malloc(nrows * ncolmax * sizeof(MatrixType));
	this->indexs = (int*)malloc(nrows * ncolmax * sizeof(int));
	if (rows == NULL) {
		this->nrows = 0;
	}

	for (int i = 0; i < nrows; i++) {
		this->rows[i] = new SparseRow(ncolmax, values + (i * ncolmax),
											indexs + (i * ncolmax));
	}
	this->count = (int*)malloc(sizeof(int));

	// Too lazy for ton of checks
	// if null pointer exception on
	// this line then allocation failed
	this->count[0] = 1;
}

//SparseMatrix::SparseMatrix(int nrows) {
//	this->nrows = nrows;
//	this->rows = (SparseRow**)malloc(nrows * sizeof(SparseRow*));
//	if (rows == NULL) {
//		this->nrows = 0;
//	}
//
//	for (int i = 0; i < nrows; i++) {
//		this->rows[i] = NULL;
//	}
//	this->count = (int*)malloc(sizeof(int));
//
//	// Too lazy for ton of checks
//	// if null pointer exception on
//	// this line then allocation failed
//	this->count[0] = 1;
//}

SparseMatrix::SparseMatrix(const SparseMatrix& other) {
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
        free(count);
        free(values);
        free(indexs);
    }

    this->count = other.count;
    this->count++;
    this->nrows = other.nrows;
    this->rows = other.rows;

    return *this;
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
        free(count);
        free(values);
        free(indexs);
	}
}

