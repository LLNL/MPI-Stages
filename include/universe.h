#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

#include <vector>
#include <unordered_map>

#include "abstract/progress.h"
#include "abstract/transport.h"
#include "abstract/checkpoint.h"
#include "pool.h"
#include "request.h"
#include "comm.h"
#include "errhandler.h"

namespace exampi
{

// NOTE make singleton at the moment
class Universe
{
private:
	// MPI universe owns all request objects
	MemoryPool<Request> request_pool;
	
	Universe();
	~Universe();

public:
	static Universe &get_root_universe();

	Group world_group;
	Comm world_comm;

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

	Request_ptr allocate_request();
	void deallocate_request(Request_ptr request);
};

}

#endif
