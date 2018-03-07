#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char** argv) {
	int size, rank;
	MPI_Request *req;
	MPI_Status *status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	req = (MPI_Request *) malloc(sizeof(MPI_Request) * (size - 1));
	status = (MPI_Status *) malloc(sizeof(MPI_Status) * (size - 1));

	if (rank == 0) {
		for (int i = 1; i < size; i++) {
			MPI_Irecv((void *) 0, 0, MPI_INT, i, 0, MPI_COMM_WORLD,
					&req[i - 1]);
		}

		MPI_Waitall(size - 1, req, status);

		for (int i = 1; i < size; i++) {
			MPI_Isend((void *) 0, 0, MPI_INT, i, 0, MPI_COMM_WORLD,
					&req[i - 1]);
		}
		MPI_Abort(MPI_COMM_WORLD, -1);
		MPI_Waitall(size - 1, req, status);
	} else {
		MPI_Send((void *) 0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Recv((void *) 0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status[0]);
	}

	MPI_Finalize();
	return 0;
}
