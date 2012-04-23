#pragma once
#include "Reader.h"
#include "Matrix.h"
#include "SparseMatrix.h"
#include <fstream>
#include <stdio.h>

using namespace std;

/**
 * This class is an implementation of a reader used to read the file format
 * found in ice sheet model output from Fastook. Make sure readFile() is called
 * before calling any of the get functions or only NULL will be received.
 *
 * This class will use a SparseMatrix storage format for A to preserve memory/
 * performance and will use normal Matrix storage for both x and b.
 *
 * The file format is as follows, assuming you are reading three matrixes.
 * A(NxN), x(Nx1), and b(Nx1). The x is the solution to the previous iteration
 * and can be a good estimate for the next solution.
 *
 * N
 * # of Non-Zeros In row
 * (Index Row) (Index Column) (Value of Element [Exponential notation])
 * ... ^^ occurs for each element in row
 * ... ^^ occurs for each row in A
 * (x[0]) (x[1]) (x[2]) ... (x[n-1])
 * (b[0]) (b[1]) (b[2]) ... (b[n-1])
 *
 */
class FastookReader : public Reader {
public:
	/**
	 * Constructs a Fastook Reader that will read from file and generate A, x,
	 * and b.
	 *
	 * This is used when you want to read a file and don't know number of
	 * non-zeros in A.
	 */
	FastookReader(const char * file) {
		this->file.open(file);
		this->maxRow = -1;
		this->A = NULL;
		this->x = this->b = NULL;
	}

	/**
	 * Constructs a Fastook Reader that will read from file and generate A, x,
	 * and b.
	 *
	 * This is used when you want to read a file and know the maximum non-zeros
	 * in any row of A.
	 *
	 * @input maxRow The maximum number of elements in a given row
	 */
	FastookReader(const char * file, int maxRow) {
		this->file.open(file);
		this->maxRow = maxRow;
		this->A = NULL;
		this->x = this->b = NULL;
	}

	/**
	 * This closes the file if not already open and frees the memofy for A, x,
	 * and b if allocated.
	 */
	~FastookReader() {
		this->file.close();
		if (this->A != NULL) {
			delete this->A;
		}
		if (this->b != NULL) {
			delete this->b;
		}
		if (this->x != NULL) {
			delete this->x;
		}
	}

	/**
	 * This will perform the reading of the file specified in the constructor.
	 * Will store the information for A, x, and b, which can be gotten using the
	 * getA(), getX(), and getB() functions after this has been called.
	 *
	 * This function will take a noticeable amount of time for larger matrixes/
	 * files.
	 */
	void readFile() {
		int rows;
		string line;
		int count = 0;

		getline(file, line);
		sscanf(line.c_str(), "%d", &rows);
		if (maxRow != -1) {
			cout << "Creating Matrix size: " << rows << " " << maxRow << endl;
			A = new SparseMatrix(rows, maxRow);
		} else {
			cout << "Creating Matrix size: " << rows << " " << endl;
			A = new SparseMatrix(rows, 10);
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
//			if (maxRow == -1) {
//				A->setRow(r, new SparseRow(rsize + 1));
//			}

			// Loop through each non-zero in the row
			for (int ct = 0; ct < rsize; ct++) {
				int cIndex, rIndex;
				float val;
				getline(file, line);
				sscanf(line.c_str(), "%d %d %g", &rIndex, &cIndex, &val);
				if ((rIndex - 1) != r) {
					printf("Row Inconsistency: %d %d\n", r, rIndex - 1);
					exit(1);
				}

				if (val != 0) {
					A->getRowS(rIndex-1)->addVal(cIndex-1, (MatrixType)val);
				}
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

	/**
	 * Get the A matrix data after readFile() has been called. Will be deleted
	 * upon deconstruction of the object.
	 */
    Matrix *getA() {
		return A;
	}

	/**
	 * Get the x matrix data after readFile() has been called. Will be deleted
	 * upon deconstruction of the object.
	 */
    Matrix *getX() {
		return x;
	}

	/**
	 * Get the b matrix data after readFile() has been called. Will be deleted
	 * upon deconstruction of the object.
	 */
    Matrix *getB() {
		return b;
	}
private:
	ifstream file;
	int maxRow;
	Matrix *x, *b;
	SparseMatrix *A;
};
