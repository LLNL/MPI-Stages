#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Checkpoint_write = PMPIX_Checkpoint_write

int PMPIX_Checkpoint_write(void) {
	int rc = exampi::global::interface->MPIX_Checkpoint_write();
	return rc;
}

}
