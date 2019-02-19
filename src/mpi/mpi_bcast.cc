#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Bcast = PMPI_Bcast

	int PMPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
	               MPI_Comm comm)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Bcast(buf, count, datatype,
		         root, comm);
		return rc;
	}

}
