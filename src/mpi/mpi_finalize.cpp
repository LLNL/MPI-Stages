#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"

extern "C"
{

#pragma weak MPI_Finalize = PMPI_Finalize

int PMPI_Finalize(void) {
	int rc = exampi::global::interface->MPI_Finalize();
  //delete exampi::basic::Interface::global;
  (void)rc;
	return MPI_SUCCESS;
}

}

