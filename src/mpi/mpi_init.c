#include "mpi.h"
#include "aumpi.h"

int MPI_Init(int *argc, char ***argv) {
	int rc = mpi_init();
	return MPI_SUCCESS;
}
