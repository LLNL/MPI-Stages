#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Comm_dup = PMPI_Comm_dup

	int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Comm_dup(comm, newcomm);
		return rc;
	}

}
