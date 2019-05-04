#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize_handler_register = PMPIX_Deserialize_handler_register

	int PMPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Deserialize_handler_register(
		           handler);
	}

}
