#include "mpi.h"
#include "ExaMPI.h"

int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
	int rc = bint.MPI_Send(buf, count, datatype, dest, tag, comm);
	return MPI_SUCCESS;
}
