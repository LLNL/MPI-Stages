#include "request.h"

namespace exampi
{

//thread_local std::mutex thr_request_lock;
thread_local std::condition_variable thr_request_condition;

Request::Request() :
	condition(nullptr),
	complete(false),
	cancelled(false),
	//persistent(false),
	active(false),
	error(MPI_SUCCESS)
{
	;
}

Request::Request(Operation op, Payload payload, Envelope envelope) :
	Request()
{
	operation = op;
	payload = payload;
	envelope = envelope;
}

void Request::release()
{
	std::lock_guard<std::mutex> lock(guard);
	debug("request lock acquired");

	// lock may or may not be acquired
	complete = true;

	// check if anyone is waiting on it
	if(condition != nullptr)
	{
		// lock is already acquired
		debug("notifying waiting thread");
		condition->notify_one();
		//condition->notify_all();
	}

	debug("released request entirely");
}

}
