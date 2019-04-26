#include "mpi.h"

#include "interfaces/interface.h"

extern "C" {

#pragma weak MPI_Barrier = PMPI_Barrier
	int PMPI_Barrier(MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Barrier(comm);
	}
}
