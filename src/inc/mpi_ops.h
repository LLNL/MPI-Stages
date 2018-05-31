#ifndef _MPI_OPS_H
#define _MPI_OPS_H

namespace exampi
{
  void internal_MAX_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
  void internal_MIN_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
  void internal_SUM_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
  void internal_MAX_LOC_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
  void internal_MIN_LOC_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt);
}

#endif
