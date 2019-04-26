#include <mpi.h>

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Comm_dup = PMPI_Comm_dup

	int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Comm_dup(comm, newcomm);
	}

}
