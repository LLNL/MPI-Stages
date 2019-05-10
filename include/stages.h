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

#ifndef __EXAMPI_STAGES_H
#define __EXAMPI_STAGES_H

#include "debug.h"
#include "errors.h"
#include "faulthandler.h"

namespace exampi
{

//#ifdef MPI_STAGES

#define CHECK_STAGES_ERROR() 	debug("checking mpi stages error state"); \
								Universe &u = Universe::get_root_universe(); \
								if (u.mpi_stages) {FaultHandler &faulthandler = FaultHandler::get_instance();\
															if(faulthandler.isErrSet()) {return MPIX_TRY_RELOAD;}}
//else {return MPI_SUCCESS;}


//#else

//#define CHECK_STAGES_ERROR()

//#endif

}

#endif
