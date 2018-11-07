#ifndef __EXAMPI_PROMISE_H
#define __EXAMPI_PROMISE_H

namespace exampi
{

// simple condition variable wrapper for threshold events (i.e. edge triggered)
class Latch : public exampi::i::Latch
{
private:
	size_t needed;
	size_t level;
	std::condition_variable cv;
	std::mutex m;

	void checkWakeup()
	{
		if(level >= needed)
			cv.notify_all();
	}
public:
	Latch(size_t n) : needed(n), level(0) {;}

	bool isOpen()
	{
		return level >= needed;
	}

	void wait()
	{
		if(isOpen()) return;
		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, [this] { return level >= needed; } );
	}

	void signal()
	{
		std::lock_guard<std::mutex> lock(m);
		level++;
		checkWakeup();
	}
};


class LatchSet

	template <typename R>
class Promise
{
private:
	size_t refcount;
	std::condition_variable cv;
	bool ready;
	R data;
	std::mutex m;
public:
	Promise() : refcount(0), ready(false) {;}
	// copy, move

};




} // ::namespace

#endif
