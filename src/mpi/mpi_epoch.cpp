#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"

extern "C"
{

int MPI_Epoch(int *out) {
	(*out) = exampi::global::epoch;
  return MPI_SUCCESS;
}

}

