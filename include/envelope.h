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

#ifndef __EXAMPI_ENVELOPE_H
#define __EXAMPI_ENVELOPE_H

#include "mpi.h"

namespace exampi
{

struct Envelope
{
	int epoch;
	int context;
	int source;
	int destination;
	int tag;
};

}

#endif
