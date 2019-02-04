#include "mpi.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Waitall = PMPI_Waitall

	int PMPI_Waitall(int count, MPI_Request array_of_requests[],
	                 MPI_Status array_of_statuses[])
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Waitall(count,
		         array_of_requests,
		         array_of_statuses);
		return rc;
	}

}
