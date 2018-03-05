#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"


extern "C"
{

#pragma weak MPI_Wait = PMPI_Wait

int PMPI_Wait(MPI_Request *request, MPI_Status *status)
{
	exampi::global::interface->MPI_Wait(request, status);
	return MPI_SUCCESS;
}

}
