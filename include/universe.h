#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

#include "pool.h"
#include "request.h"

namespace exampi
{

// TODO make singleton at the moment
class Universe
{
private:
	// MPI universe owns all request objects
	MemoryPool<Request> request_pool;

	Universe()
	{
		exampi::groups.push_back(group);

		communicator = new Comm(true, group, group);
		communicator->set_rank(exampi::rank);
		communicator->set_context(0, 1);

		exampi::communicators.push_back(communicator);
	}


public:

	static Universe &get_root_universe()
	{
		static Universe root;
		
		return root;
	}
	
	// prevent Universe from being copied
	Universe(const Universe &u)				= delete;
	Universe &operator=(const Universe &u)	= delete;

	// hold communicators
	// hold groups
	// ...


	~Universe()
	{
		for(auto &&com : exampi::communicators)
	  	{
        	delete com;
	    }
        exampi::communicators.clear();
       
        // delete groups
		for (auto &&group : exampi::groups)
        {
        	delete group;
        }
        exampi::groups.clear();
	}

	std::unique_ptr<Request> allocate_request()
	{
		return request_pool.allocate();
	}
};
#endif
