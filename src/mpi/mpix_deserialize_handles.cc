#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize_handles = PMPIX_Deserialize_handles

	int PMPIX_Deserialize_handles()
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Deserialize_handles();
	}

}
