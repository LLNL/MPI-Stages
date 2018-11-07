#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Serialize_handler_register = PMPIX_Serialize_handler_register

	int PMPIX_Serialize_handler_register(const MPIX_Serialize_handler handler)
	{
		int rc = exampi::interface->MPIX_Serialize_handler_register(handler);
		return rc;
	}

}
