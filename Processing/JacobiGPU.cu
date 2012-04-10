
typedef float MatrixType;
__global__ void computeIter(int maxRow, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs);


void computeIterations(int maxRow, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs) {
	computeIter<<<1, 1>>>(maxRow, dXs, dNextXs, dIndexs, dRowVals, dRowInds, dYs);
}

__global__ void computeIter(int maxRow, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs) {

}
