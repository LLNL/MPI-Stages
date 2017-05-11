#include "mpi.h"
#include "ExaMPI.h"

int MPI_Init(int *argc, char ***argv) {
	int rc = bint.MPI_Init(argc, argv);
	return MPI_SUCCESS;
}
