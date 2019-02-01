#ifndef __EXAMPI_REQUEST_H
#define __EXAMPI_REQUEST_H

#include <mutex>
#include <condition_variable>

#include "mpi.h"
#include "envelope.h"
#include "payload.h"

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
	std::mutex lock;
	std::condition_variable *condition;

	bool complete;
	bool cancelled;

	bool persistent;
	bool active;

	// MPI data
	Operation operation;
	Envelope envelope;
	Payload payload;

	Request(Operation operation, Payload payload, Envelope envelope);
};

}

#endif
