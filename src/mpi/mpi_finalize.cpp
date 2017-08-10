#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

#ifdef __cplusplus
extern "C"
{
#endif

int MPI_Finalize(void) {
	int rc = exampi::bint.MPI_Finalize();
	return MPI_SUCCESS;
}

#ifdef __cplusplus
}
#endif
