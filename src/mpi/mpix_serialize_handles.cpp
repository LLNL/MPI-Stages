#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Serialize_handles = PMPIX_Serialize_handles

	int PMPIX_Serialize_handles()
	{
		int rc = exampi::BasicInterface::get_instance()->MPIX_Serialize_handles();
		return rc;
	}

}
