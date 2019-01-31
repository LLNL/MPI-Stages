#include "request.h"

namespace exampi
{

thread_local std::mutex thr_request_lock;
thread_local std::condition_variable thr_request_condition;

Request::Request() : condition(nullptr), complete(false)
{
	;
}

}
