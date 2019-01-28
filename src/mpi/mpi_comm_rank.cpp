#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Comm_rank = PMPI_Comm_rank

	int PMPI_Comm_rank(MPI_Comm c, int *r)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Comm_rank(c, r);
		return rc;
	}

}
