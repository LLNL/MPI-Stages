#ifndef __EXAMPI_ASYNC_H
#define __EXAMPI_ASYNC_H

#include "ExaMPI.h"
#include "pool.h"

namespace exampi
{

template<typename T>
class AsyncQueue
{
protected:
	//MemoryPool<std::promise<T>> promise_pool;

	std::list<T> data;

	std::list<std::unique_ptr<std::promise<T>>> promises;
	//std::list<typename MemoryPool<std::promise<T>>::unique_ptr> promises;

	std::mutex promiseLock;
	std::mutex dataLock;
	std::condition_variable pcond;

	void test()
	{
		debug("AQ:  testing");

		std::unique_lock<std::mutex> lock(promiseLock);

		// blocks until >0 promises and data
		// FIXME this only ever allows a single slot in AsyncQueue
		pcond.wait(lock, [&]() -> bool { return (promises.size() > 0 && data.size() > 0); });

		if(promises.size() > 0)
			if(data.size() > 0)
			{
				debug("AQ:  data and promises, advancing");

				// MR 22/01/19 calling thread does copy between threads?
				promises.front()->set_value(std::move(data.front()));

				promises.pop_front();

				if (!data.empty())
					data.pop_front();
			}

		lock.unlock();

		debug("AQ:  done testing");
	}

public:
	//AsyncQueue() : promise_pool(256)
	AsyncQueue()
	{
		debug("AsyncQueue:  constructing");
	}

	std::future<T> promise()
	{
		debug("AQ: Promise requested.  data(" << data.size() << ") promises(" <<
		        promises.size() << ")");

		// TODO MR 22/01/19 avoid lock constantly
		std::unique_lock<std::mutex> lock(promiseLock);

		// NOTE is this two new operations?
		promises.push_back(make_unique<std::promise<T>>());
		//promises.push_back(this->promise_pool.alloc());

		debug("AQ: Promise pushed; about to get_future...");

		auto result = promises.back()->get_future();

		lock.unlock();
		pcond.notify_all();

		return result;
	}

	void put(T &&v)
	{
		debug("Inserting into aqueue");

		// push unique ptr into linked list
		data.push_back(std::move(v));

		// XXX MR 22/01/19 why test every time?
		test();
	}
};

}

#endif
