/*
   mpi_ops.cpp 

   define public built-in ops;
   define internal implementation;

   provide easy map for functions that use these to get the correct fn pointer to the type specific version.

*/

#include "mpi.h"
#include "mpi_ops.h"

/* #include "ExaMPI.h" */
/* #include "basic.h"*/

extern "C" 
{
  /* these generics don't actually do anything; they are just names for the C/C++ user-level API;

    
  */ 


}

namespace exampi
{

  template <typename T>
  void internal_max_op_fn( T *invec, T *inoutvec, int *len)
  {
    for(int i = 0; i < *len; ++i)
    {
      if(invec[i] > inoutvec[i]) inoutvec[i] = invec[i];
    }
  }

  template <typename T>
  void internal_min_op_fn( T *invec, T *inoutvec, int *len)
  {
    for(int i = 0; i < *len; ++i)
    {
      if(invec[i] < inoutvec[i]) inoutvec[i] = invec[i];
    }
  }

  template <typename T>
  void internal_sum_op_fn( T *invec, T *inoutvec, int *len)
  {
    for(int i = 0; i < *len; ++i)
    {
      inoutvec[i] += invec[i];
    }
  }


  void internal_MAX_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt)
  {
    switch(*dt)
    {
    case MPI_BYTE:
      internal_max_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_CHAR:
      internal_max_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_UNSIGNED_CHAR:
      internal_max_op_fn<unsigned char>((unsigned char *)invec, (unsigned char *)inoutvec, len);
      break;

    case MPI_SHORT:
      internal_max_op_fn<short>((short *)invec, (short *)inoutvec, len);
      break;

    case MPI_UNSIGNED_SHORT:
      internal_max_op_fn<unsigned short>((unsigned short *)invec, (unsigned short *)inoutvec, len);
      break;

    case MPI_INT:
      internal_max_op_fn<int>((int *)invec, (int *)inoutvec, len);
      break;

    case MPI_UNSIGNED_INT:
      internal_max_op_fn<unsigned int>((unsigned int *)invec, (unsigned int *)inoutvec, len);
      break;

    case MPI_LONG:
      internal_max_op_fn<long>((long *)invec, (long *)inoutvec, len);
      break;

    case MPI_UNSIGNED_LONG:
      internal_max_op_fn<unsigned long>((unsigned long *)invec, (unsigned long *)inoutvec, len);
      break;

    case MPI_FLOAT:
      internal_max_op_fn<float>((float *)invec, (float *)inoutvec, len);
      break;

    case MPI_DOUBLE:
      internal_max_op_fn<double>((double *)invec, (double *)inoutvec, len);
      break;


    default:
      break;
    }
  }

  void internal_MIN_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt)
  {
    switch(*dt)
    {
    case MPI_BYTE:
      internal_min_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_CHAR:
      internal_min_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_UNSIGNED_CHAR:
      internal_min_op_fn<unsigned char>((unsigned char *)invec, (unsigned char *)inoutvec, len);
      break;

    case MPI_SHORT:
      internal_min_op_fn<short>((short *)invec, (short *)inoutvec, len);
      break;

    case MPI_UNSIGNED_SHORT:
      internal_min_op_fn<unsigned short>((unsigned short *)invec, (unsigned short *)inoutvec, len);
      break;

    case MPI_INT:
      internal_min_op_fn<int>((int *)invec, (int *)inoutvec, len);
      break;

    case MPI_UNSIGNED_INT:
      internal_min_op_fn<unsigned int>((unsigned int *)invec, (unsigned int *)inoutvec, len);
      break;

    case MPI_LONG:
      internal_min_op_fn<long>((long *)invec, (long *)inoutvec, len);
      break;

    case MPI_UNSIGNED_LONG:
      internal_min_op_fn<unsigned long>((unsigned long *)invec, (unsigned long *)inoutvec, len);
      break;

    case MPI_FLOAT:
      internal_min_op_fn<float>((float *)invec, (float *)inoutvec, len);
      break;

    case MPI_DOUBLE:
      internal_min_op_fn<double>((double *)invec, (double *)inoutvec, len);
      break;


    default:
      break;
    }
  }

  void internal_SUM_OP(void *invec, void *inoutvec, int *len, MPI_Datatype *dt)
  {
    switch(*dt)
    {
    case MPI_BYTE:
      internal_sum_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_CHAR:
      internal_sum_op_fn<char>((char *)invec, (char *)inoutvec, len);
      break;

    case MPI_UNSIGNED_CHAR:
      internal_sum_op_fn<unsigned char>((unsigned char *)invec, (unsigned char *)inoutvec, len);
      break;

    case MPI_SHORT:
      internal_sum_op_fn<short>((short *)invec, (short *)inoutvec, len);
      break;

    case MPI_UNSIGNED_SHORT:
      internal_sum_op_fn<unsigned short>((unsigned short *)invec, (unsigned short *)inoutvec, len);
      break;

    case MPI_INT:
      internal_sum_op_fn<int>((int *)invec, (int *)inoutvec, len);
      break;

    case MPI_UNSIGNED_INT:
      internal_sum_op_fn<unsigned int>((unsigned int *)invec, (unsigned int *)inoutvec, len);
      break;

    case MPI_LONG:
      internal_sum_op_fn<long>((long *)invec, (long *)inoutvec, len);
      break;

    case MPI_UNSIGNED_LONG:
      internal_sum_op_fn<unsigned long>((unsigned long *)invec, (unsigned long *)inoutvec, len);
      break;

    case MPI_FLOAT:
      internal_sum_op_fn<float>((float *)invec, (float *)inoutvec, len);
      break;

    case MPI_DOUBLE:
      internal_sum_op_fn<double>((double *)invec, (double *)inoutvec, len);
      break;


    default:
      break;
    }
  }

}
