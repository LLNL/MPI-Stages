#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Checkpoint = PMPIX_Checkpoint

int PMPIX_Checkpoint(void) {
	exampi::global::interface->MPIX_Checkpoint();
    //(*p) = exampi::global::epoch;
  //delete exampi::basic::Interface::global;
	return MPI_SUCCESS;
}

}
