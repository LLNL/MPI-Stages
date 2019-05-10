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

#ifndef __EXAMPI_INTERFACE_TRANSPORT_H
#define __EXAMPI_INTERFACE_TRANSPORT_H

#include <fstream>
#include <map>
#include <memory>

#include "header.h"
#include "request.h"
#include "protocol.h"
#include "abstract/stages.h"

namespace exampi
{

struct Transport: virtual public Stages
{
	virtual ~Transport() {};

	// ordered map (preference) of protocol initator and maximum message size
	// note chose size_t over long int, because -1 == inf would work
	//      but it cuts down by a large range, max size_t is enough
	virtual const std::map<Protocol, size_t> &provided_protocols() const = 0;

	// receive a header or nullptr
	virtual Header_uptr ordered_recv() = 0;

	// fill header into matching request
	virtual void fill(Header_uptr, Request *) = 0;

	// send request with protocol
	virtual void reliable_send(const Protocol, const Request *) = 0;
};

} // ::exampi

#endif
