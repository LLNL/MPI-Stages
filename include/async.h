#ifndef __EXAMPI_ASYNC_H
#define __EXAMPI_ASYNC_H

#include "ExaMPI.h"

namespace exampi
{

template<typename T>
class AsyncQueue
{
protected:
	std::list<T> data;

	std::list<std::unique_ptr<std::promise<T>>> promises;

	std::mutex promiseLock;
	std::mutex dataLock;
	std::condition_variable pcond;

	void test()
	{
		debugpp("AQ:  testing");

		// XXX MR what does this test?

		std::unique_lock<std::mutex> lock(promiseLock);

		// blocks until >0 promises and data
		pcond.wait(lock, [&]() -> bool { return (promises.size() > 0 && data.size() > 0); });

		if(promises.size() > 0)
			if(data.size() > 0)
			{
				debugpp("AQ:  data and promises, advancing");

				// MR 22/01/19 calling thread does comms between threads?
				promises.front()->set_value(std::move(data.front()));

				promises.pop_front();

				if (!data.empty())
					data.pop_front();
			}

		lock.unlock();

		debugpp("AQ:  done testing");
	}

public:
	AsyncQueue()
	{
		debugpp("AsyncQueue:  constructing");
	}

	std::future<T> promise()
	{
		debugpp("AQ: Promise requested.  data(" << data.size() << ") promises(" << promises.size() << ")");
		
		// TODO MR 22/01/19 avoid lock constantly

		std::unique_lock<std::mutex> lock(promiseLock);
		
		// TODO MR 22/01/19 replace with MemoryPool
		promises.push_back(make_unique<std::promise<T>>());

		debugpp("AQ: Promise pushed; about to get_future...");

		auto result = promises.back()->get_future();
		lock.unlock();
		pcond.notify_all();

		return result;
	}

	void put(T &&v)
	{
		debugpp("Inserting into aqueue");
		
		// push unique ptr into linked list
		data.push_back(std::move(v));

		// XXX MR 22/01/19 why test every time?
		test();
	}
};

}

#endif
