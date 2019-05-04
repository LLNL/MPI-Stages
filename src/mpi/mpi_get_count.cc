#include <mpi.h>

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Get_count = PMPI_Get_count

	int PMPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Get_count(status,
		        datatype, count);
	}

}
