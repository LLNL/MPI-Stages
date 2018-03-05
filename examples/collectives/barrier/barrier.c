#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char** argv) {
	int i, size, rank;
	const int n_iters = 10000;
	const int max_wait_in_sec = 4;
	double t, elapsed;
	int wait_in_sec;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("%d entering MPI_Barrier\n", rank);
	MPI_Barrier(MPI_COMM_WORLD);
	printf("%d passed MPI_Barrier\n", rank);

	if (rank == size - 1) {
		wait_in_sec = max_wait_in_sec;
	}
	else {
		srand(rank);
		wait_in_sec = rand() % (max_wait_in_sec + 1);
	}
	printf("%d entering MPI_Barrier validation test\n", rank);
	t = MPI_Wtime();
	MPI_Barrier(MPI_COMM_WORLD);
	sleep(wait_in_sec);
	MPI_Barrier(MPI_COMM_WORLD);
	elapsed = MPI_Wtime() - t;
	printf("elapsed time %lf", elapsed);

	if (fabs(elapsed - max_wait_in_sec) < 1.0) {
		printf("validation passed. err=%lf sec\n", elapsed - max_wait_in_sec);
	}
	else {
		printf("validation failed. err=%lf sec\n", elapsed - max_wait_in_sec);
	}

	printf("%d entering MPI_Barrier\n", rank);
	t = MPI_Wtime();
	for (i = 0; i < n_iters; i++) {
		MPI_Barrier(MPI_COMM_WORLD);
	}
	elapsed = MPI_Wtime() - t;

	printf("Average Barrier latency = %lf usec\n", elapsed/(double) n_iters*1000.0*1000.0);

	MPI_Finalize();
	return 0;
}
