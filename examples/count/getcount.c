#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	const int MAX_NUMBERS = 100;
	int numbers[MAX_NUMBERS];
	int number_amount;
	int count;
	if (rank == 0) {
		srand(time(NULL));
		number_amount = (rand() / (float)RAND_MAX) * MAX_NUMBERS;

		MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
		printf("0 sent %d numbers to 1\n", number_amount);
	}
	else if (rank == 1) {
		MPI_Status status;
		MPI_Recv(numbers, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		MPI_Get_count(&status, MPI_INT, &count);

		printf("1 received %d numbers from 0. Message source = %d, tag = %d\n", count, status.MPI_SOURCE, status.MPI_TAG);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}
