#ifndef __EXAMPI_PROTOCOL_QUEUE_H
#define __EXAMPI_PROTOCOL_QUEUE_H

#include <mutex>
#include <list>

#include "debug.h"
#include "request.h"
#include "pool.h"

namespace exampi
{

class ProtocolQueue
{
private:
	std::mutex queue_lock;

	std::list<Request *> slots;
	
public:
	ProgressQueue();
	~ProgressQueue();
	
	void insert(Request *request);
	void remove(Request *request);
};

}

#endif
