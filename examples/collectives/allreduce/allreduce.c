#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define ARRAY_LEN 2

int main(int argc, char** argv) {
	int i, size, rank;
	int msg[ARRAY_LEN], recvmsg[ARRAY_LEN];
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	for (int i = 0; i < ARRAY_LEN; i++) {
		msg[i] = i + 1;
	}

	MPI_Allreduce(msg, recvmsg, ARRAY_LEN, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	printf("Sum is %d\t%d\n", recvmsg[0], recvmsg[1]);

	MPI_Finalize();
	return 0;
}
