#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Serialize_handler_register = PMPIX_Serialize_handler_register

	int PMPIX_Serialize_handler_register(const MPIX_Serialize_handler handler)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPIX_Serialize_handler_register(
		           handler);
	}

}
