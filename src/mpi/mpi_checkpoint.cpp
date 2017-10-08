#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"

extern "C"
{

int MPI_Checkpoint(int *p) {
	exampi::global::interface->MPI_Checkpoint(p);
  (*p) = exampi::global::epoch;
  //delete exampi::basic::Interface::global;
	return MPI_SUCCESS;
}

}
