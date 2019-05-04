#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Irecv = PMPI_Irecv

	int PMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	               MPI_Comm comm, MPI_Request *request)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Irecv(buf, count, datatype,
		                                     dest, tag,
		                                     comm, request);
	}

}
