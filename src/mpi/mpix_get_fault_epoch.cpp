#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{
#pragma weak MPIX_Get_fault_epoch = PMPIX_Get_fault_epoch

	int PMPIX_Get_fault_epoch(int *out)
	{
		int rc = exampi::BasicInterface::get_instance().MPIX_Get_fault_epoch(out);
		return rc;
	}

}

