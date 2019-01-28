#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize_handler_register = PMPIX_Deserialize_handler_register

	int PMPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler)
	{
		int rc = exampi::BasicInterface::get_instance().MPIX_Deserialize_handler_register(
		             handler);
		return rc;
	}

}
