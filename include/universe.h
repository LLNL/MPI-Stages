#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

#include <vector>
#include <unordered_map>
#include <exception>

#include "abstract/progress.h"
#include "abstract/transport.h"
#include "abstract/checkpoint.h"
#include "pool.h"
#include "request.h"
#include "comm.h"

namespace exampi
{

class UniverseEnvironmentException: public std::exception
{
	const char *what() const noexcept override
	{
		return "Universe failed to find required environment variables.";
	}
};

// note: make singleton at the moment
class Universe
{
private:
	// MPI universe owns all request objects
	MemoryPool<Request> request_pool;

	Universe();
	~Universe();

public:
	static Universe &get_root_universe();

	std::shared_ptr<Group>	world_group;
	std::shared_ptr<Comm>	world_comm;

	std::vector<std::shared_ptr<Comm>> 	communicators;
	std::vector<std::shared_ptr<Group>> groups;

	std::unordered_map<MPI_Datatype, Datatype> datatypes;

	int rank;
	int world_size;
	int epoch;
	std::string epoch_config;

	std::unique_ptr<Progress> progress;

	// TODO this will be removed for Stages Consolidation
	std::unique_ptr<Checkpoint> checkpoint;

	// todo eventually Interface *interface

	// prevent Universe from being copied
	Universe(const Universe &u)				= delete;
	Universe &operator=(const Universe &u)	= delete;

	Request_ptr allocate_request();
	void deallocate_request(Request_ptr request);

	// mpi stages
	int save(std::ostream &t);
	int load(std::istream &t);
	void halt();
};

}

#endif
