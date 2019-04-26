#include <mpi.h>

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Comm_rank = PMPI_Comm_rank

	int PMPI_Comm_rank(MPI_Comm c, int *r)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Comm_rank(c, r);
	}

}
