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

#ifndef __EXAMPI_DECIDER_H
#define __EXAMPI_DECIDER_H

#include "protocol.h"
#include "universe.h"
#include "request.h"

namespace exampi
{

struct Decider
{
	virtual Protocol decide(const Request_ptr request,
	                        const Universe &universe) const = 0;
};

}

#endif
