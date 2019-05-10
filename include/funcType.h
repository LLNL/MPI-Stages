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

#ifndef __EXAMPI_FUNCTYPE_H
#define __EXAMPI_FUNCTYPE_H

#include <map>
#include <mpi_ops.h>

namespace exampi
{
typedef std::map<MPI_Op, MPI_User_Function> funcType;
const funcType functions =
{
	{std::make_pair(MPI_SUM, &internal_SUM_OP)},
	{std::make_pair(MPI_MAX, &internal_MAX_OP)},
	{std::make_pair(MPI_MIN, &internal_MIN_OP)},
	{std::make_pair(MPI_MAXLOC, &internal_MAX_LOC_OP)},
	{std::make_pair(MPI_MINLOC, &internal_MIN_LOC_OP)},
};
}

#endif
