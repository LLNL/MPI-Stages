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

#include <memory>

#include "universe.h"
#include "engines/blockingprogress.h"
#include "daemon.h"
#include "interfaces/interface.h"
#include "interfaces/mpistages.h"

namespace exampi
{

Universe &Universe::get_root_universe()
{
	static Universe root;

	return root;
}

Universe::Universe() : request_pool(20), initialized(false)
{
	debug("creating universe");

	// todo migrate check out of MPI_Init
	// check that exampi-mpiexec was used to launch the application
	//if(std::getenv("EXAMPI_MONITORED") == NULL)
	//{
	//	debug("Application was not launched with mpiexec.");
	//	return MPI_ERR_MPIEXEC;
	//}
	//debug("MPI_Init passed EXAMPI_LAUNCHED check.");

	char *variable;

	// parse EXAMPI_RANK environment variable
	variable = std::getenv("EXAMPI_RANK");
	if(variable == nullptr)
		throw std::runtime_error("Universe is not able to find EXAMPI_RANK");
	rank = std::stoi(std::string(variable));
	debug("rank " << rank);

	// parse EXAMPI_EPOCH_FILE environment variable
	variable = std::getenv("EXAMPI_EPOCH_FILE");
	if(variable == nullptr)
		throw std::runtime_error("Universe is not able to find EXAMPI_EPOCH_FILE");
	epoch_config = std::string(variable);
	debug("epoch file " << epoch_config);

	// parse EXAMPI_EPOCH environment variable
	variable = std::getenv("EXAMPI_EPOCH");
	if(variable == nullptr)
		throw std::runtime_error("Universe is not able to find EXAMPI_EPOCH");
	epoch = std::stoi(std::string(variable));
	debug("epoch " << epoch);

	// parse EXAMPI_WORLD_SIZE environment variable
	variable = std::getenv("EXAMPI_WORLD_SIZE");
	if(variable == nullptr)
		throw std::runtime_error("Universe is not able to find EXAMPI_WORLD_SIZE");
	world_size = std::stoi(std::string(variable));
	debug("world size " << world_size);

	// parse EXAMPI_STAGES environment variable
	mpi_stages = std::getenv("EXAMPI_STAGES");
	debug("mpi stages enabled " << mpi_stages);

	if (mpi_stages)
		interface = std::make_unique<StagesInterface>();
	else
		interface = std::make_unique<BasicInterface>();

	debug("finished creating universe");
}

Universe::~Universe()
{
	this->finalize();

	debug("terminating universe");
}

void Universe::initialize()
{
	initialized = true;
}

void Universe::finalize()
{
	debug("universe finalize called");

	initialized = false;

	debug("progress engine being removed");
	Progress *engine = progress.get();
	progress.release();

	delete engine;

	debug("deleting all communicators");
	communicators.clear();

	debug("deleting all groups");
	groups.clear();
}

Request_ptr Universe::allocate_request()
{
	debug("allocating request from memory pool");
	return request_pool.allocate();
}

void Universe::deallocate_request(Request_ptr request)
{
	debug("freeing request to memory pool");
	request_pool.deallocate(request);
}

}
