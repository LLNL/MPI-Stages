#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Checkpoint_read = PMPIX_Checkpoint_read

	int PMPIX_Checkpoint_read()
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Checkpoint_read();
	}

}
