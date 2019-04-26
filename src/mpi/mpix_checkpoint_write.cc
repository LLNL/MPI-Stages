#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Checkpoint_write = PMPIX_Checkpoint_write

	int PMPIX_Checkpoint_write(void)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Checkpoint_write();
	}

}
