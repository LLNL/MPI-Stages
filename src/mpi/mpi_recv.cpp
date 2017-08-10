#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Status *status) {
	int rc = exampi::bint.MPI_Recv(buf, count, datatype, dest, tag, comm, NULL);
	return MPI_SUCCESS;
}
