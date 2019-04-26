#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Send = PMPI_Send

	int PMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Send(buf, count, datatype,
		         dest, tag,
		         comm);
	}

}
