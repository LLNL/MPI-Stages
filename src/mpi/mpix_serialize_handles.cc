#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Serialize_handles = PMPIX_Serialize_handles

	int PMPIX_Serialize_handles()
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Serialize_handles();
	}

}
