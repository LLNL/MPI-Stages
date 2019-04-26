#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Wtime = PMPI_Wtime

	double PMPI_Wtime(void)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Wtime();
	}

}
