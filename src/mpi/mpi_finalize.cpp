#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"

extern "C"
{

#pragma weak MPI_Finalize = PMPI_Finalize

int PMPI_Finalize(void) {
	int rc = exampi::basic::Interface::global->MPI_Finalize();
  delete exampi::basic::Interface::global;
	return MPI_SUCCESS;
}

}

