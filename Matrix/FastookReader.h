#pragma once
#include "Reader.h"
#include "Matrix.h"
#include "SparseMatrix.h"
#include <fstream>
#include <stdio.h>

using namespace std;

class FastookReader : public Reader {
public:
	FastookReader(const char * file) {
		this->file.open(file);
		this->maxRow = -1;
		this->A = NULL;
		this->x = this->b = NULL;
	}

	FastookReader(const char * file, int maxRow) {
		this->file.open(file);
		this->maxRow = maxRow;
		this->A = NULL;
		this->x = this->b = NULL;
	}

	~FastookReader() {
		this->file.close();
		delete this->A;
		delete this->b;
		delete this->x;
	}

	void readFile() {
		int rows;
		string line;

		getline(file, line);
		sscanf(line.c_str(), "%d", &rows);
		if (maxRow != -1) {
			cout << "Creating Matrix size: " << rows << " " << maxRow << endl;
			A = new SparseMatrix(rows, maxRow);
		} else {
			cout << "Creating Matrix size: " << rows << " " << endl;
			A = new SparseMatrix(rows);
		}
		cout << "Creating Matrix size: " << rows << " " << endl;
		x = new Matrix(rows, 1);
		cout << "Creating Matrix size: " << rows << " " << endl;
		b = new Matrix(rows, 1);

		// Loop through each row
		for (int r = 0; r < rows; r++) {
			int rsize;

			getline(file, line);
			sscanf(line.c_str(), "%d", &rsize);
			if (maxRow == -1) {
				A->setRow(r, new SparseRow(rsize));
			}

			// Loop through each non-zero in the row
			for (int ct = 0; ct < rsize; ct++) {
				int cIndex, rIndex;
				MatrixType val;
				getline(file, line);
				sscanf(line.c_str(), "%d %d %g", &rIndex, &cIndex, &val);

				A->getRow(rIndex-1).addVal(cIndex-1, val);
			}
		}
		cout << "Done getting A" << endl;

		MatrixType value;
		// Loop over the x vector
		for (int r = 0; r < rows; r++) {
			file >> value;
			x->setVal(r, 0, value);
		}
		cout << "Done getting x" << endl;

		// Loop over the b vector
		for (int r = 0; r < rows; r++) {
			file >> value;
			b->setVal(r, 0, value);
		}
		cout << "Done getting b" << endl;
		file.close();
	}

    Matrix *getA() {
		return A;
	}

    Matrix *getX() {
		return x;
	}

    Matrix *getB() {
		return b;
	}
private:
	ifstream file;
	int maxRow;
	Matrix *x, *b;
	SparseMatrix *A;
};
