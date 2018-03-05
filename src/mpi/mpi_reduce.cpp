#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"


extern "C"
{
#pragma weak MPI_Reduce = PMPI_Reduce

int PMPI_Reduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm) {
	int rc = exampi::global::interface->MPI_Reduce(s_buf, r_buf, count, type, op, root, comm);
  (void)rc;
	return rc;
}

}
