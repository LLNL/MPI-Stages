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

#ifndef __EXAMPI_HEADER_H
#define __EXAMPI_HEADER_H

#include <functional>

#include "protocol.h"
#include "envelope.h"

namespace exampi
{

struct Header
{
	Protocol protocol;
	Envelope envelope;
};

typedef std::unique_ptr<Header, std::function<void(Header *)>> Header_uptr;

}

#endif
