#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"


extern "C"
{

#pragma weak MPI_Init = PMPI_Init

int PMPI_Init(int *argc, char ***argv) {
  exampi::basic::Interface::global = new exampi::basic::Interface();
	int rc = exampi::basic::Interface::global->MPI_Init(argc, argv);
	return MPI_SUCCESS;
}

}
