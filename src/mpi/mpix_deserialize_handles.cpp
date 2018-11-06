#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize_handles = PMPIX_Deserialize_handles

	int PMPIX_Deserialize_handles()
	{
		int rc = exampi::global::interface->MPIX_Deserialize_handles();
		return rc;
	}

}
