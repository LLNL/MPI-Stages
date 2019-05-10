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

#ifndef __EXAMPI_REQUEST_H
#define __EXAMPI_REQUEST_H

#include <mutex>
#include <condition_variable>

#include "mpi.h"
#include "envelope.h"
#include "payload.h"
#include "protocol.h"

namespace exampi
{

enum class Operation: int
{
	Send,
	Ssend,
	Bsend,
	Rsend,

	Receive

	//AllReduce
	//AllGather
	//...
};

// thread waiting mechanism
extern thread_local std::condition_variable thr_request_condition;

struct Request
{
	// management data
	std::mutex guard;
	std::condition_variable *condition;

	volatile bool complete;
	bool cancelled;
	bool persistent;
	bool active;
	bool freed;

	int error;

	// MPI data
	Operation operation;
	Protocol protocol;

	// protocol message
	Envelope envelope;

	// note will need extending once we do more complex things
	Payload payload;

	Request();
	// p2p constructor
	Request(Operation operation, Payload payload, Envelope envelope);
	// collective constructor
	// rma constructor

	void release();
};

typedef Request *Request_ptr;

}

#endif
