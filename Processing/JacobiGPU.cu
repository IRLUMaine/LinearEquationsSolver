#include <stdio.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>

#define RTHRESHOLD .00001
#define THRESHOLD .00001

//#define FINDEX

#define DISP 50001
//126976
//167940

//#define single(x) if (loc == DISP) x
#define single(x)

typedef float MatrixType;
__global__ void computeIter(int id, int maxRow, int num, int iter,
		MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, int *dRowInds, MatrixType *dYs, int *dDiffs);

#ifdef FINDEX
__global__ void rearrange(int id, int maxRow, int num, int iter,
		MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int *dDiffs);
#endif

void rearrangeCall(int id, int varPBlock, int maxRow, int num, int size,
		int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int* dDiffs) {
#ifdef FINDEX
	int blocks = (num / (varPBlock));
	cudaError_t error;

	if ((blocks * (varPBlock)) != num)
		blocks++;
	if (varPBlock != 1024)
		printf("INCORRECT PARAMETER varPBlock VALID VALUES=1024\n");

	printf("Rearranging...");
	rearrange<<<blocks, 1024, 12288>>>(id, maxRow, num, iter, dXs, dNextXs,
			dIndexs, dRowVals, dRowVRearr, dRowInds, dRowIRearr, dYs, dDiffs);
	if ((error = cudaThreadSynchronize()) != cudaSuccess) {
		printf("Rearrange Error %d: %s\n", id, cudaGetErrorString(error));
		exit(1);
		return;
	}
	printf("Done\n");
#endif
}

void computeIterations(int id, int varPBlock, int maxRow, int num, int size,
		int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int* dDiffs) {
	int blocks = (num / (varPBlock));
	cudaError_t error;

	if ((blocks * (varPBlock)) != num)
		blocks++;
	if (varPBlock != 1024)
		printf("INCORRECT PARAMETER varPBlock VALID VALUES=1024\n");

#ifdef FINDEX
	computeIter<<<blocks, 1024, 12288>>>(id, maxRow, num, iter, dXs, dNextXs,
			dIndexs, dRowVRearr, dRowIRearr, dYs, dDiffs);
#else
	computeIter<<<blocks, 1024, 12288>>>(id, maxRow, num, iter, dXs, dNextXs,
			dIndexs, dRowVals, dRowInds, dYs, dDiffs);
#endif

	if ((error = cudaThreadSynchronize()) != cudaSuccess) {
		printf("Error %d: %s\n", id, cudaGetErrorString(error));
		exit(1);
		return;
//    } else {
//    	printf("Iter\n");
	}
}

#ifdef FINDEX
__global__ void rearrange(int gid, int maxRow, int num, int iter,
		MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, MatrixType* dRowVRearr, int *dRowInds,
		int *dRowIRearr, MatrixType *dYs, int *dDiffs) {
	__shared__ int shared[3072];
	int id = threadIdx.x;
	int loc = blockDim.x * blockIdx.x + threadIdx.x;
	if (loc >= num) {
		return;
	}
	// Each iteration x val for this block
	int *inds = &shared[1024];

	// diffs for last iteration
	int *diffs = (int*) &shared[2048];

	// index in the x array for the changing vars
	int localIndex[20];
	MatrixType localCoeff[20];

	// index in the x array for the static vars
	int staticIndex[20];
	MatrixType staticCoeff[20];
	MatrixType staticVar0;
	//MatrixType staticVar1;

	// global index of this thread
	// local index of this thread
	// row to operate on
	int ind = dIndexs[loc];
	//printf("loc:%d id:%d ind:%d %p\n", loc, id, ind, dIndexs);

	inds[id] = ind;
	// addresses for sparse row data
	MatrixType* dRowV = dRowVals + (loc * maxRow);
	int* dRowI = dRowInds + (loc * maxRow);

	// counting
	int i, j;
	// flaggging
	int flag;
	// temp stuff
	int temp;
	// localIndex counting
	int ct = 0;
	// staticIndex counting
	int sct = 0;
	__syncthreads();
	diffs[0] = 0;

	// setup statics
	for (i = 0; dRowI[i] != -1; i++) {
		if (dRowI[i] == ind) {
			staticVar0 = dRowV[i];
		}
	}
	__syncthreads();
	// search for statics / dynamics
	for (j = 0; (j < maxRow) && (dRowI[j] != -1); j++) {
		temp = dRowI[j];
		if (temp != ind) {
			flag = 1;
			for (i = 0; i < 1024; i++) {
				if (temp == inds[i]) {
					flag = 0;
					localCoeff[ct] = dRowV[j];
					localIndex[ct++] = i;
					break;
				}
			}
			if (flag) {
				staticCoeff[sct] = dRowV[j];
				staticIndex[sct++] = -temp - 1;
			}
		}
	}
	localIndex[ct] = -1;
	staticIndex[sct] = 1;
	__syncthreads();
	dRowIRearr[loc] = ct + sct;
	dRowVRearr[loc] = staticVar0;
	sct = loc + num;
	for (ct = 0; localIndex[ct] != -1; ct++) {
		dRowIRearr[sct] = localIndex[ct];
		dRowVRearr[sct] = localCoeff[ct];
		sct += num;
	}
	__syncthreads();
	for (ct = 0; staticIndex[ct] != 1; ct++) {
		dRowIRearr[sct] = staticIndex[ct];
		dRowVRearr[sct] = staticCoeff[ct];
		sct += num;
	}
}
#endif

#define NWEIGHT 1
#define LWEIGHT 4

__global__ void computeIter(int gid, int maxRow, int num, int iter,
		MatrixType *dXs, MatrixType *dNextXs, int *dIndexs,
		MatrixType *dRowVals, int *dRowInds, MatrixType *dYs, int *dDiffs) {
	__shared__ int shared[3072];
	int loc = blockDim.x * blockIdx.x + threadIdx.x;
	// local index of this thread
	int id = threadIdx.x;
	if (loc < num) {
		// Each iteration x val for this block
		MatrixType *x = (MatrixType*) &shared[0];
		int *inds = &shared[1024];

		// diffs for last iteration
		int *diffs = (int*) &shared[2048];

		// index in the x array for the changing vars
		int localIndex[20];
		MatrixType localCoeff[20];

		// Local x value
		MatrixType lx;

		// variables that are not changing
		// first is the A[i][i] diag val
		// second is a value that will be precalculated
		// for the vars that aren't changing
		MatrixType staticVar0;
		MatrixType staticVar1;

		// global index of this thread
		// row to operate on
		int ind = dIndexs[loc];
		//printf("loc:%d id:%d ind:%d %p\n", loc, id, ind, dIndexs);

		inds[id] = ind;

		// counting
		int i, j;
		MatrixType tempF;
		// localIndex counting
		int ct = 0;
		lx = x[id] = dXs[ind];
		__syncthreads();
		diffs[0] = 0;

		staticVar1 = dYs[loc];
		__syncthreads();

#ifdef FINDEX
		int nvars = dRowInds[loc];
		int dInd = loc + num;
		staticVar0 = dRowVals[loc];
		for (i = 0; i < nvars; i++) {

			int curIndex = dRowInds[dInd];
			MatrixType curValue = dRowVals[dInd];

			if (curIndex >= 0) {
				localCoeff[ct] = curValue;
				localIndex[ct++] = curIndex;
			} else {
				staticVar1 -= curValue * dXs[-curIndex - 1];
				staticVar1 -= 1;
			}
			dInd += num;
		}
#else
		// addresses for sparse row data
		MatrixType* dRowV = dRowVals + (loc * maxRow);
		int* dRowI = dRowInds + (loc * maxRow);
		// temp stuff
		int temp;
		// flaggging
		int flag;

		// search for statics / dynamics
		for (j = 0; (j < maxRow) && (dRowI[j] != -1); ++j) {
			temp = dRowI[j];
			if (temp != ind) {
				flag = 1;
				for (i = 0; i < 1024; ++i) {
					if (temp == inds[i]) {
						flag = 0;
						localCoeff[ct] = dRowV[j];
						localIndex[ct++] = i;
						break;
					}
				}
				if (flag) {
					staticVar1 -= dRowV[j] * dXs[temp];
				}
			} else {
				staticVar0 = dRowV[j];
			}
		}
#endif
		localIndex[ct] = -1;

		// last iteration need to store the diffs for convergence
		// checking
		tempF = staticVar1;
		for (j = 0; localIndex[j] != -1; ++j) {
			tempF -= localCoeff[j] * x[localIndex[j]];
		}
		tempF /= staticVar0;
		tempF = (NWEIGHT * tempF + LWEIGHT * lx) / (NWEIGHT + LWEIGHT);
		if (fabs(lx - tempF) > (RTHRESHOLD * fabs(tempF) + THRESHOLD)) {
			diffs[0] = 1;
		}
		lx = x[id] = tempF;
		__syncthreads();

		// time for real calculations... damn
		// yes this loop runs iter - 1 times
		// first time requires diff calcs
		for (i = 1; i < iter; i++) {
			tempF = staticVar1;
			for (j = 0; localIndex[j] != -1; ++j) {
				tempF -= localCoeff[j] * x[localIndex[j]];
			}
			//tempF += staticVar1;
			tempF /= staticVar0;
			tempF = (NWEIGHT * tempF + LWEIGHT * lx) / (NWEIGHT + LWEIGHT);
			lx = x[id] = tempF;
			__syncthreads();
		}

		dXs[ind] = x[id];
		if (id == 0) {
			dDiffs[blockIdx.x] = diffs[0];
		}
	}
}
