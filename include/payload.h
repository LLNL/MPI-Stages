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

#ifndef __EXAMPI_PAYLOAD_H
#define __EXAMPI_PAYLOAD_H

#include "datatype.h"

namespace exampi
{

struct Payload
{
	const void *buffer;
	//MPI_Datatype datatype;
	Datatype *datatype;
	int count;
};

}

#endif
