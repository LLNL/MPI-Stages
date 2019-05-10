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

#ifndef __EXAMPI_ABSTRACT_PROGRESS_H
#define __EXAMPI_ABSTRACT_PROGRESS_H

#include <mpi.h>

#include "request.h"
#include "abstract/stages.h"

namespace exampi
{

class Progress: virtual public Stages
{
public:
	virtual ~Progress() {}

	// handle user request object
	virtual void post_request(Request *request) = 0;
};

} // ::exampi

#endif // __EXAMPI_PROGRESS_H
