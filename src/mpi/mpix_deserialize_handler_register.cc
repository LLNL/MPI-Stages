// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

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
