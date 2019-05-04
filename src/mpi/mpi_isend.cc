#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Isend = PMPI_Isend

	int PMPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	               MPI_Comm comm, MPI_Request *request)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Isend(buf, count, datatype,
		                                     dest, tag,
		                                     comm, request);
	}

}
