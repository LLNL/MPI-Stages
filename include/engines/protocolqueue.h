#ifndef __EXAMPI_PROTOCOL_QUEUE_H
#define __EXAMPI_PROTOCOL_QUEUE_H

#include <mutex>

#include "debug.h"
#include "request.h"
#include "pool.h"

namespace exampi
{

class ProgressQueue
{
private:
	std::mutex lock;

	std::vector<MemoryPool<Request>::unique_ptr> slots;
	
public:
	ProgressQueue();
	~ProgressQueue();
	
	void insert(Request);
	void remove(Request);
};

}

#endif
