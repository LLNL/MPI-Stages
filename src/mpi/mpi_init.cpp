#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

int MPI_Init(int *argc, char ***argv) {
	int rc = exampi::bint.MPI_Init(argc, argv);
	return MPI_SUCCESS;
}
