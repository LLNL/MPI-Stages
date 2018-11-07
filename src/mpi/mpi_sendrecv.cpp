#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Sendrecv = PMPI_Sendrecv

	int PMPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
	                  int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype,
	                  int source, int recvtag, MPI_Comm comm, MPI_Status *status)
	{
		int rc = exampi::interface->MPI_Sendrecv(sendbuf, sendcount, sendtype,
			         dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status);
		(void)rc;
		return rc;
	}

}
