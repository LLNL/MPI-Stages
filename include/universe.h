// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

#include <vector>
#include <unordered_map>
#include <exception>

#include "abstract/progress.h"
//#include "abstract/transport.h"
#include "abstract/interface.h"
#include "abstract/stages.h"
#include "pool.h"
#include "request.h"
#include "communicator.h"

namespace exampi
{

//class UniverseEnvironmentException: public std::exception
//{
//	const char *what() const noexcept override
//	{
//		return "Universe failed to find required environment variables.";
//	}
//};

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

	void initialize();
	void finalize();

	std::shared_ptr<Group>	world_group;
	std::shared_ptr<Comm>	world_comm;

	std::vector<std::shared_ptr<Comm>> 	communicators;
	std::vector<std::shared_ptr<Group>> groups;

	std::unordered_map<MPI_Datatype, Datatype> datatypes;

	bool initialized;
	int rank;
	int world_size;

	int epoch;
	std::string epoch_config;

	bool mpi_stages;

	std::unique_ptr<Progress> progress;
	std::unique_ptr<Interface> interface;

	// eventually Interface *interface

	// prevent Universe from being copied
	Universe(const Universe &u)				= delete;
	Universe &operator=(const Universe &u)	= delete;

	Request_ptr allocate_request();
	void deallocate_request(Request_ptr request);
};

}

#endif
