#ifndef __EXAMPI_STAGES_H
#define __EXAMPI_STAGES_H

#include "debug.h"
#include "errors.h"

namespace exampi
{

#ifdef MPI_STAGES

#define CHECK_STAGES_ERROR() debugpp("checking mpi stages error state"); if(exampi::handler->isErrSet()) {return MPIX_TRY_RELOAD;}

#else

#define CHECK_STAGES_ERROR() 

#endif

}

#endif
