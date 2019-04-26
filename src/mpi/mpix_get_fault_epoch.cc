#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Get_fault_epoch = PMPIX_Get_fault_epoch

	int PMPIX_Get_fault_epoch(int *out)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Get_fault_epoch(out);
	}

}

