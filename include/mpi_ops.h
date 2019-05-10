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

#ifndef _MPI_OPS_H
#define _MPI_OPS_H

#include "mpi.h"

namespace exampi
{

void internal_MAX_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
void internal_MIN_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
void internal_SUM_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
void internal_MAX_LOC_OP(void *invec, void *inoutvec, int *len,
                         MPI_Datatype *dt);
void internal_MIN_LOC_OP(void *invec, void *inoutvec, int *len,
                         MPI_Datatype *dt);
}

#endif
