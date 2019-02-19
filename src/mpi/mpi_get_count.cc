#include <mpi.h>

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Get_count = PMPI_Get_count

	int PMPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count)
	{
		int rc =  exampi::BasicInterface::get_instance().MPI_Get_count(status,
		          datatype, count);
		return rc;
	}

}