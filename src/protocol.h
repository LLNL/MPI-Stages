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

#include "protocolqueue.h"

namespace exampi
{

ProtocolQueue::ProtocolQueue() : slots(256)
{
	;
}

ProtocolQueue::~ProtocolQueue()
{
	;
}

int ProtocolQueue::insert(Request *request)
{
	std::lock_guard<std::mutex> lock(queue_lock);

	slots.push_back(request);

	return 0;
}

int ProtocolQueue::remove(Request *request)
{
	std::lock_guard<std::mutex> lock(queue_lock);

	slots.remove(request);

	return 0;
}

}
