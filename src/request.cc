#include "request.h"

namespace exampi
{

//thread_local std::mutex thr_request_lock;
thread_local std::condition_variable thr_request_condition;

Request::Request()
{
	;
}

Request::Request(Operation operation, Payload payload, Envelope envelope) : 
	operation(operation),
	payload(payload),
	envelope(envelope)
{
	;
}

void Request::release()
{
	// lock may or may not be acquired
	complete = true;

	// check if anyone is waiting on it
	if(condition != nullptr)
	{
		// lock is already acquired
		condition->notify_one();
	}	
}

}
