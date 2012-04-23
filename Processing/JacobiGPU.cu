#include <stdio.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>

#define RTHRESHOLD .0001
#define THRESHOLD .0001


#define DISP -1

typedef float MatrixType;
__global__ void computeIter(int maxRow, int num, int size, int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs, int *dDiffs);


void computeIterations(int varPBlock, int maxRow, int num, int size, int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs, int* dDiffs) {
	int blocks = (num / (varPBlock));
    cudaError_t error;


	if ((blocks * (varPBlock)) != num) blocks++;
	if (varPBlock != 1024) printf("INCORRECT PARAMETER varPBlock VALID VALUES=1024\n");

	computeIter<<<blocks, 1024, 12288>>>(maxRow, num, size, iter, dXs, dNextXs, dIndexs, dRowVals, dRowInds, dYs, dDiffs);
    if ((error = cudaThreadSynchronize()) != cudaSuccess) {
        printf("Error: %s\n", cudaGetErrorString(error));
		exit(1);
        return;
    }
}

#define NWEIGHT 1
#define LWEIGHT 4

__global__ void computeIter(int maxRow, int num, int size, int iter, MatrixType *dXs, MatrixType *dNextXs, int *dIndexs, MatrixType *dRowVals, int *dRowInds, MatrixType *dYs, int *dDiffs) {
	__shared__ int shared[3072];
	// Each iteration x val for this block
	MatrixType *x = (MatrixType*)&shared[0];
	int *inds = &shared[1024];

	// diffs for last iteration
	int *diffs = (int*)&shared[2048];

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
	int loc = blockDim.x * blockIdx.x + threadIdx.x;
	if (loc >= num) {
		return;
	}
	// local index of this thread
	int id = threadIdx.x;
	// row to operate on
	int ind = dIndexs[loc];
	//printf("loc:%d id:%d ind:%d %p\n", loc, id, ind, dIndexs);

	inds[id] = ind;
	// addresses for sparse row data
	MatrixType* dRowV = dRowVals + (ind * maxRow);
	int* dRowI = dRowInds + (ind * maxRow);

	// counting
	int i, j;
	// flaggging
	int flag;
	// temp stuff
	int temp;
	MatrixType tempF;
	// localIndex counting
	int ct = 0;
	lx = x[id] = dXs[loc];
	__syncthreads();
	diffs[0] = 0;

	// setup statics
	for (i = 0; dRowI[i] != -1; i++) {
		if (dRowI[i] == ind) {
			staticVar0 = dRowV[i];
/*
			if (loc == DISP) {
				printf("Set A[i][i] to %lf\n", staticVar0);
			}
*/
		}
	}
	staticVar1 = dYs[ind];// / staticVar0;
/*
	if (loc == DISP) {
		printf("Set Y to %lf\n", staticVar1);
	}
*/
/*
	if (loc == 5222) {
		for (i = 0; dRowI[i] != -1; i++) {
			printf("%lf * x_%d + ", dRowV[i], dRowI[i]);
		}
		printf("\n\n");
	}
*/

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
/*
					if (loc == DISP) {
						printf("Dyn -= %lf * x_%d\n", localCoeff[ct-1], localIndex[ct-1]);
					}
*/
					break;
				}
			}
			if (flag) {
				staticVar1 -= dRowV[j] * dXs[temp];// / staticVar0;
/*
				if (loc == DISP) {
					printf("-= %lf * %lf = %lf\n", dRowV[j], dXs[temp], staticVar1);
				}
*/
			}
		}
	}
	localIndex[ct] = -1;


	// last iteration need to store the diffs for convergence
	// checking
	tempF = staticVar1;
	for (j = 0; localIndex[j] != -1; j++) {
		tempF -= localCoeff[j] * x[localIndex[j]];
	}
	//tempF += staticVar1;
	tempF /= staticVar0;
	tempF = (NWEIGHT * tempF + LWEIGHT * lx) / (NWEIGHT + LWEIGHT);
	//diffs[0] = tempF;
	if (fabs(lx - tempF) > (RTHRESHOLD * fabs(tempF) + THRESHOLD)) {

		diffs[0] = 1;//fabs(tempF - x[id]) / fabs(x[id]);
	//} else {
	//	diffs[id] = 0;
	}
	lx = x[id] = tempF;
	__syncthreads();

	// time for real calculations... damn
	// yes this loop runs iter - 1 times
	// last time requires diff calcs
	for (i = 1; i < iter; i++) {
		tempF = staticVar1;
		for (j = 0; localIndex[j] != -1; j++) {
			tempF -= localCoeff[j] * x[localIndex[j]];
		}
		//tempF += staticVar1;
		tempF /= staticVar0;
		tempF = (NWEIGHT * tempF + LWEIGHT * lx) / (NWEIGHT + LWEIGHT);
		lx = x[id] = tempF;
		__syncthreads();
	}
/*
	tempF = dYs[ind];
	if (loc == DISP) {
		printf("Val: %lf\n", tempF);
	}
	for (j = 0; dRowI[j] != -1; j++) {
		if (dRowI[j] != ind) {
			tempF -= dRowV[j] * dXs[dRowI[j]];
		} else {
			staticVar1 = dRowV[j];
		}
	if (loc == DISP) {
		printf("Val: %lf\n", tempF);
	}
	}
	tempF /= staticVar0;
	if (loc == DISP) {
		printf("Val: %lf %lf\n", tempF, dXs[ind]);
	}
	//tempF += staticVar1;
	tempF = NWEIGHT * tempF / (NWEIGHT + LWEIGHT) + LWEIGHT * dXs[ind] / (NWEIGHT + LWEIGHT);
	//diffs[0] = tempF;
	diffs[id] = fabs(tempF - dXs[ind]);/// / x[id];
	x[id] = tempF;
	__syncthreads();
*/

	// diff to max reduction
/*
	for (i = 1; i < 1024; i <<= 1) {
		if ((id & i) == 0) {
			if (diffs[id] < diffs[id + i]) {
				diffs[id] = diffs[id + i];
			}
		} else {
			break;
		}
		__syncthreads();
	}
*/
//	if (diffs[id] > 0) {
//		diffs[0] = 1;
//	}
/*
	__syncthreads();
	if (loc == DISP) {
		printf("Was: %lf Now: %lf\n", dXs[loc], x[id]);
	}
*/
	dXs[loc] = x[id];
	if (id == 0) {
		dDiffs[blockIdx.x] = diffs[0];
	}
}
