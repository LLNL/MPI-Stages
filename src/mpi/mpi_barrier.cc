#include "mpi.h"
#include "basic.h"
#include "interfaces/interface.h"

extern "C" {

#pragma weak MPI_Barrier = PMPI_Barrier
	int PMPI_Barrier(MPI_Comm comm)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Barrier(comm);
		return rc;
	}
}
