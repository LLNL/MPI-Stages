#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

#include <vector>
#include <unordered_map>

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

	std::vector<Comm *> communicators;
	std::vector<Group *> groups;
	std::unordered_map<MPI_Datatype, Datatype> datatypes;

	int rank;
	int world_size;
	int epoch;
	std::string epoch_config;

	Progress *progress;
	Transport *transport;
	Checkpoint *checkpoint;
	errHandler *errhandler;
	
	// prevent Universe from being copied
	Universe(const Universe &u)				= delete;
	Universe &operator=(const Universe &u)	= delete;

	~Universe()
	{
		for(auto &&com : communicators)
	  	{
        	delete com;
	    }
        communicators.clear();
       
        // delete groups
		for (auto &&group : groups)
        {
        	delete group;
        }
        groups.clear();
	}

	Request_ptr allocate_request()
	{
		return request_pool.allocate();
	}
};
#endif
