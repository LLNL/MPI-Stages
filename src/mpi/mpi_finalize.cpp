#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

#ifdef __cplusplus
extern "C"
{
#endif

int MPI_Finalize(void) {
	int rc = exampi::BasicInterface::global->MPI_Finalize();
  delete exampi::BasicInterface::global;
	return MPI_SUCCESS;
}

#ifdef __cplusplus
}
#endif
