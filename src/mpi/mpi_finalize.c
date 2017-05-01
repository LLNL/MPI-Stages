#include "mpi.h"
#include "aumpi.h"

int MPI_Finalize(void) {
	int rc = mpi_finalize();
	return MPI_SUCCESS;
}
