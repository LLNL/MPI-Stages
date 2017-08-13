#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

int MPI_Init(int *argc, char ***argv) {
  exampi::BasicInterface::global = new exampi::BasicInterface();
	int rc = exampi::BasicInterface::global->MPI_Init(argc, argv);
	return MPI_SUCCESS;
}
