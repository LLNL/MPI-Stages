#include "mpi.h"
#include "aumpi.h"

int MPI_Finalize(void) {
	int rc = bint.MPI_Finalize();
	return MPI_SUCCESS;
}
