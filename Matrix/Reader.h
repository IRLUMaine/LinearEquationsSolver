#pragma once
#include "Matrix.h"


/**
 * This is an interface class that will be
 * used to interface to various types of
 * sparse/non-sparse matrix file formats
 * they will always return some class
 * Matrix
 */
class Reader {
public:
	virtual ~Reader() {
	}

	/*
	 * This function will read in the input file
	 * and produce the appropriate type of matrix
	 */
	virtual void readFile() = 0;
	/*
	 * These functions can be called after readFile()
	 * to retrieve the data from the file.
	 */
	virtual Matrix *getA() = 0;
	virtual Matrix *getX() = 0;
	virtual Matrix *getB() = 0;
};
