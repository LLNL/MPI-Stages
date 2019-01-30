#include "protocolqueue.h"

namespace exampi
{

ProtocolQueue::ProtocolQueue()
{
	slots.reserve(128);
}

ProtocolQueue::~ProtocolQueue()
{

}

ProtocolQueue::insert(Request* request)
{
	std::lock_guard(queue_lock);
	
	slots.push_back(request);
}

ProtocolQueue::remove(Request *request)
{
	std::lock_guard(queue_lock);
	
	slots.remove(request);
}

}
