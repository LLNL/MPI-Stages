#ifndef __EXAMPI_FUNCTYPE_H
#define __EXAMPI_FUNCTYPE_H

#include <map>
#include <mpi_ops.h>

namespace exampi
{
  typedef std::map<MPI_Op, MPI_User_Function> funcType;
  const funcType functions = {
		  {std::make_pair(MPI_SUM, &internal_SUM_OP)},
		  {std::make_pair(MPI_MAX, &internal_MAX_OP)},
		  {std::make_pair(MPI_MIN, &internal_MIN_OP)},
		  {std::make_pair(MPI_MAXLOC, &internal_MAX_LOC_OP)},
		  {std::make_pair(MPI_MINLOC, &internal_MIN_LOC_OP)},
  };
}

#endif
