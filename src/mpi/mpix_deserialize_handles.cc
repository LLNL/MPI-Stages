#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize_handles = PMPIX_Deserialize_handles

	int PMPIX_Deserialize_handles()
	{
		int rc = exampi::BasicInterface::get_instance().MPIX_Deserialize_handles();
		return rc;
	}

}
