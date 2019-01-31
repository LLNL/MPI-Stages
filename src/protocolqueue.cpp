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

void ProtocolQueue::insert(Request* request)
{
	std::lock_guard<std::mutex> lock(queue_lock);
	
	slots.push_back(request);
}

void ProtocolQueue::remove(Request *request)
{
	std::lock_guard<std::mutex> lock(queue_lock);
	
	slots.remove(request);
}

}
