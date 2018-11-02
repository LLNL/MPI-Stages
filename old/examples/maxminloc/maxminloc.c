#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char **argv) {
	double ain[30], aout[30];
	int ind[30];
	struct {
		double val;
		int rank;
	}in[30], out[30];

	int i, myrank, root;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	srand(myrank);
	for (i = 0; i < 30; ++i) {
		ain[i] = (double)(rand() % 100);
		printf("Value %lf\n", ain[i]);
	}
	for (i = 0; i < 30; ++i) {
		in[i].val = ain[i];
		in[i].rank = myrank;
	}

	MPI_Reduce(in, out, 30, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

	if (myrank == 0) {
		for (i = 0; i < 30; ++i) {
			aout[i] = out[i].val;
			ind[i] = out[i].rank;
			printf("MAX value %d with %lf\n", ind[i], aout[i]);
		}
	}

	MPI_Finalize();
	return 0;
}
