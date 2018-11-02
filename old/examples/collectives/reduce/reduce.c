#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int myrank, numprocs;

/* just a function to waste some time */
float work() {
	float x, y;
	if (myrank % 2) {
		for (int i = 0; i < 100000000; ++i) {
			x = i / 0.001;
			y += x;
		}
	} else {
		for (int i = 0; i < 100000; ++i) {
			x = i / 0.001;
			y += x;
		}
	}
	return y;
}

int main(int argc, char **argv) {
	int node;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &node);

	printf("Hello World from Node %d\n", node);

	/*variables used for gathering timing statistics*/
	double mytime, maxtime, mintime, avgtime;

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Barrier(MPI_COMM_WORLD); /*synchronize all processes*/

	mytime = MPI_Wtime(); /*get time just before work section */
	work();
	mytime = MPI_Wtime() - mytime; /*get time just after work section*/

	/*compute max, min, and average timing statistics*/
	MPI_Reduce(&mytime, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&mytime, &mintime, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&mytime, &avgtime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	/* plot the output */
	if (myrank == 0) {
		avgtime /= numprocs;
		printf("Min: %lf  Max: %lf  Avg:  %lf\n", mintime, maxtime, avgtime);
	}

	MPI_Finalize();

	return 0;
}
