#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

#ifdef __cplusplus
extern "C"
{
#endif

#pragma weak MPI_Finalize = PMPI_Finalize

int PMPI_Finalize(void) {
	int rc = exampi::BasicInterface::global->MPI_Finalize();
  delete exampi::BasicInterface::global;
	return MPI_SUCCESS;
}

#ifdef __cplusplus
}
#endif
