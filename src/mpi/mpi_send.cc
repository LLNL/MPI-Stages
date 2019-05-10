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

#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Send = PMPI_Send

	int PMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Send(buf, count, datatype,
		                                    dest, tag,
		                                    comm);
	}

}
