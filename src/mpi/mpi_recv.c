#include "mpi.h"
#include "ExaMPI.h"

int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Status *status) {
	int rc = bint.MPI_Recv(buf, count, datatype, dest, tag, comm, &status);
	return MPI_SUCCESS;
}
