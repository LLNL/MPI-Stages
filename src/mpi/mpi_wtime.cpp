#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Wtime = PMPI_Wtime

	double PMPI_Wtime(void)
	{
		double ret = exampi::global::interface->MPI_Wtime();
		return ret;
	}

}
