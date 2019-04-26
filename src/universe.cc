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

Universe::Universe() : request_pool(128), initialized(false)
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
		throw UniverseEnvironmentException();
	rank = std::stoi(std::string(variable));
	debug("rank " << rank);

	// parse EXAMPI_EPOCH_FILE environment variable
	variable = std::getenv("EXAMPI_EPOCH_FILE");
	if(variable == nullptr)
		throw UniverseEnvironmentException();
	epoch_config = std::string(variable);
	debug("epoch file " << epoch_config);

	// parse EXAMPI_EPOCH environment variable
	variable = std::getenv("EXAMPI_EPOCH");
	if(variable == nullptr)
		throw UniverseEnvironmentException();
	epoch = std::stoi(std::string(variable));
	debug("epoch " << epoch);

	// parse EXAMPI_WORLD_SIZE environment variable
	variable = std::getenv("EXAMPI_WORLD_SIZE");
	if(variable == nullptr)
		throw UniverseEnvironmentException();
	world_size = std::stoi(std::string(variable));
	debug("world size " << world_size);

	// parse MPI_STAGES environment variable
	mpi_stages = std::getenv("MPI_STAGES");
	debug("mpi stages" << mpi_stages);

	if (mpi_stages)
		interface = std::make_unique<StagesInterface>();
	else
		interface = std::make_unique<BasicInterface>();

	debug("finished creating universe");
}

Universe::~Universe()
{
	debug("universe being destroyed, deleting all communicators");
	communicators.clear();

	debug("deleting all groups");
	groups.clear();

	debug("terminating universe");
}

void Universe::initialize()
{
	initialized = true;
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
