#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Waitall = PMPI_Waitall

	int PMPI_Waitall(int count, MPI_Request array_of_requests[],
	                 MPI_Status array_of_statuses[])
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();

		return universe.interface->MPI_Waitall(count,
		                                       array_of_requests,
		                                       array_of_statuses);
	}

}
