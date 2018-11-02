#include "point.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define MPIX_SAFE_CALL(__operation,__predicate,__label) {int __code = __operation; if(__code != MPI_SUCCESS) {__predicate; goto __label;}}
#define MAX_ITER 3

int main_loop(int argc, char **argv, int epoch, int *done);
void Application_Checkpoint_Read(int epoch, int rank, point& pt);
void Application_Checkpoint_Write(int epoch, int rank, point& pt);
/*struct point {
        int x;
        int y;
}pt;
MPI_Request *rreq;
MPI_Request *sreq;*/

int commRecv(point& pt) {
        int myRank;
        int code = MPI_SUCCESS;
        MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &myRank), code = MPIX_TRY_RELOAD, fail_return);

        pt.rreq = (MPI_Request *) calloc(6, sizeof(MPI_Request));
        if (myRank == 0) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 1) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 2) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 3) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 4) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 5) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 6) {
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }

       return code;
       fail_return:
	   	   return MPIX_TRY_RELOAD;
}

int commSend(point& pt) {
        int myRank;
        int code = MPI_SUCCESS;
        MPI_Status status[6];
        MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &myRank), code = MPIX_TRY_RELOAD, fail_return);

        pt.sreq = (MPI_Request *) calloc(6, sizeof(MPI_Request));
        if (myRank == 0) {
        	int val = pt.x + 1;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 1) {
        	int val = pt.x + 2;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 2) {
        	int val = pt.x + 3;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 3) {
        	int val = pt.y + 1;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 4) {
        	int val = pt.y + 2;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 5) {
        	int val = pt.y + 3;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 6) {
        	int val = pt.y + 4;
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[4]), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[5]), code = MPIX_TRY_RELOAD, fail_return);
        }

        printf("Before Waitall\n");
        MPIX_SAFE_CALL(MPI_Waitall(6, pt.sreq, status), code = MPIX_TRY_RELOAD, fail_return);

        return code;
        fail_return:
			return MPIX_TRY_RELOAD;
}


int commWait(point& pt) {
        MPI_Status status;
        int myRank;
        int code = MPI_SUCCESS;
        MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &myRank), code = MPIX_TRY_RELOAD, fail_return);

        if (myRank == 0) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);

        }
        if (myRank == 1) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 2) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 3) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 4) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 5) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }
        if (myRank == 6) {
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[0], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[1], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[2], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[3], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[4], &status), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPI_Wait(&pt.rreq[5], &status), code = MPIX_TRY_RELOAD, fail_return);
        }

        return code;
        fail_return:
		return MPIX_TRY_RELOAD;
}

int compute(point& pt) {
	int code = MPI_SUCCESS;
	MPIX_SAFE_CALL(commRecv(pt), code = MPIX_TRY_RELOAD, fail_return);
	MPIX_SAFE_CALL(commSend(pt), code = MPIX_TRY_RELOAD, fail_return);
	MPIX_SAFE_CALL(commWait(pt), code = MPIX_TRY_RELOAD, fail_return);

	return code;
	fail_return:
	return MPIX_TRY_RELOAD;
}

void Application_Checkpoint_Read(int epoch, int rank, point& pt) {
	read(epoch, rank, pt);
}
void Application_Checkpoint_Write(int epoch, int rank, point& pt) {
	write(epoch, rank, pt);
}

int main(int argc, char** argv)
{
	int abort = 0;
	int done = 0;
	int code = MPI_SUCCESS;

	int fault_epoch = 0;

	while (!abort && !done) {
		switch(code) {
		case MPI_SUCCESS: /*process create/recreate situation*/
			MPI_Init(&argc, &argv); /*Function name might change later*/
			MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
			break;
		case MPIX_TRY_RELOAD:
			code = MPIX_Load_checkpoint(); /*read the second to last checkpointed MPI state for this process and clear pending communication*/
			break;
		default:
			abort = 1;
			break;
		}
		if (abort)
			goto abort_process;

		MPIX_Get_fault_epoch(&fault_epoch);
		code = main_loop(argc, argv, fault_epoch, &done);

		abort_process:
		if (abort) {
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
	}
	MPI_Finalize();
	return 0;
}

int main_loop(int argc, char **argv, int epoch, int *done) {
        int size, rank;
        point *pt;
        int failure;
        MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
        MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);

        if (epoch > 0) {
        	failure = 20;
        	pt = new point();
        	Application_Checkpoint_Read(epoch - 1, rank, *pt);
        }
        else {
        	failure = 2;
        	pt = new point(1, 2);
        	pt->setIter(0);
        	MPIX_SAFE_CALL(commRecv(*pt), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(commSend(*pt), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(commWait(*pt), code = MPIX_TRY_RELOAD, fail_return);

        	MPIX_SAFE_CALL(MPI_Barrier(MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
        	printf("End of Initialization\n");
        }
        int code = MPI_SUCCESS;

        for (int i = pt->getIter(); i < MAX_ITER; i++) {
        	MPIX_SAFE_CALL(compute(*pt), code = MPIX_TRY_RELOAD, fail_return);
        	if (rank == 0 && failure == i) {
        		exit(123);
        	}
        	MPIX_SAFE_CALL(MPI_Barrier(MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
        	printf("End of iteration %d ", i);
        	printf("value of struct: %d, %d", pt->x, pt->y);
        	pt->setIter(pt->getIter() + 1);
        	Application_Checkpoint_Write(epoch, rank, *pt);
        	MPIX_SAFE_CALL(MPIX_Checkpoint(), code = MPIX_TRY_RELOAD, fail_return);
        	MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
        }

        if (code == MPI_SUCCESS) {
        	*done = 1;
        }
        return code;

        fail_return:
		return MPIX_TRY_RELOAD;
}










/**************************/





#include "point.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#define MAX_ITER 2
/*struct point {
        int x;
        int y;
}pt;
MPI_Request *rreq;
MPI_Request *sreq;*/

void commRecv(point& pt) {
        int myRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
        pt.rreq = (MPI_Request *) calloc(6, sizeof(MPI_Request));
        if (myRank == 0) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
        if (myRank == 1) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
        if (myRank == 2) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
       if (myRank == 3) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
        if (myRank == 4) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
        if (myRank == 5) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
       if (myRank == 6) {
                MPI_Irecv(&pt.x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.rreq[0]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.rreq[1]);
                MPI_Irecv(&pt.x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.rreq[2]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.rreq[3]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.rreq[4]);
                MPI_Irecv(&pt.y, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.rreq[5]);
        }
}

void commSend(point& pt) {
        int myRank;
        MPI_Status status[6];
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
        pt.sreq = (MPI_Request *) calloc(6, sizeof(MPI_Request));
        if (myRank == 0) {
                int val = pt.x + 1;
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 1) {
                int val = pt.x + 2;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 2) {
                int val = pt.x + 3;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 3) {
                int val = pt.y + 1;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 4) {
                int val = pt.y + 2;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 5) {
                int val = pt.y + 3;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 6, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }
        if (myRank == 6) {
                int val = pt.y + 4;
                MPI_Isend(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &pt.sreq[0]);
                MPI_Isend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &pt.sreq[1]);
                MPI_Isend(&val, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &pt.sreq[2]);
                MPI_Isend(&val, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &pt.sreq[3]);
                MPI_Isend(&val, 1, MPI_INT, 4, 0, MPI_COMM_WORLD, &pt.sreq[4]);
                MPI_Isend(&val, 1, MPI_INT, 5, 0, MPI_COMM_WORLD, &pt.sreq[5]);
        }

        printf("Before Waitall\n");
        MPI_Waitall(6, pt.sreq, status);
}

void commWait(point& pt) {
        MPI_Status status;
        int myRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

        if (myRank == 0) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 1) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 2) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 3) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 4) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 5) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
        if (myRank == 6) {
                MPI_Wait(&pt.rreq[0], &status);
                MPI_Wait(&pt.rreq[1], &status);
                MPI_Wait(&pt.rreq[2], &status);
                MPI_Wait(&pt.rreq[3], &status);
                MPI_Wait(&pt.rreq[4], &status);
                MPI_Wait(&pt.rreq[5], &status);
        }
}

void compute(point& pt) {
        commRecv(pt);
        commSend(pt);
        commWait(pt);
}

int main(int argc, char **argv) {
        int size, rank;
        point *pt;
        int iter = 0;
        pt = new point(1, 2);

        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size); // 3 process, everyone communicating with everyone using non-blocking calls
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        commRecv(*pt);
        commSend(*pt);
        commWait(*pt);

        MPI_Barrier(MPI_COMM_WORLD);
        printf("End of initialization\n");

        for (int i = iter; i < MAX_ITER; i++) {
                compute(*pt);
                MPI_Barrier(MPI_COMM_WORLD);
                printf("End of iteration %d ", i);
                printf("value of struct: %d, %d", pt->x, pt->y);
        }
        //printf("value of struct: %d, %d", pt->x, pt->y);
        //free(pt->rreq);
        //pt->rreq = NULL;
        //free(pt->sreq);
        //pt->sreq = NULL;
        return 0;
}

