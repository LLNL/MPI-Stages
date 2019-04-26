#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Comm_size = PMPI_Comm_size

	int PMPI_Comm_size(MPI_Comm c, int *r)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Comm_size(c, r);
	}


}

