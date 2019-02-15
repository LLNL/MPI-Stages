#include <memory>

#include "universe.h"
#include "checkpoints/checkpoint.h"
#include "engines/blockingprogress.h"
#include "daemon.h"

namespace exampi
{

Universe &Universe::get_root_universe()
{
	static Universe root;

	return root;
}

Universe::Universe() : request_pool(128)
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

	debug("creating checkpoint");
	checkpoint = std::make_unique<BasicCheckpoint>();
	// todo mpi stages progress should be created here

	// MPI WORLD GROUP
	debug("generating world group");

	// NOTE this potentially becomes huge! millions++
	std::list<int> rankList;
	for(int idx = 0; idx < world_size; ++idx)
		rankList.push_back(idx);

	world_group = std::make_shared<Group>(rankList);
	groups.push_back(world_group);

	// MPI_COMM_WORLD
	debug("generating world communicator");

	world_comm = std::make_shared<Comm>();

	world_comm->is_intra = true;
	world_comm->local = world_group;
	world_comm->remote = world_group;
	world_comm->set_rank(rank);
	world_comm->set_context(0, 1);

	communicators.push_back(world_comm);

	debug("generating universe datatypes");
	datatypes =
	{
		{ MPI_BYTE, Datatype(MPI_BYTE,           sizeof(unsigned char),  true,  true, true)},
		{ MPI_CHAR, Datatype(MPI_CHAR,           sizeof(char),           true,  true, true)},
#if 0
		{ MPI_WCHAR, Datatype(MPI_WCHAR,          sizeof(wchar_t),        true,  true, true)},
#endif
		{ MPI_UNSIGNED_CHAR,  Datatype(MPI_UNSIGNED_CHAR,  sizeof(unsigned char),  true,  true, true)},
		{ MPI_SHORT,          Datatype(MPI_SHORT,          sizeof(short),          true,  true, true)},
		{ MPI_UNSIGNED_SHORT, Datatype(MPI_UNSIGNED_SHORT, sizeof(unsigned short), true,  true, true)},
		{ MPI_INT,            Datatype(MPI_INT,            sizeof(int),            true,  true, true)},
		{ MPI_UNSIGNED_INT,   Datatype(MPI_UNSIGNED_INT,   sizeof(unsigned int),   true,  true, true)},
		{ MPI_LONG,           Datatype(MPI_LONG,           sizeof(long),           true,  true, true)},
		{ MPI_UNSIGNED_LONG,  Datatype(MPI_UNSIGNED_LONG,  sizeof(unsigned long),  true,  true, true)},
		{ MPI_FLOAT,          Datatype(MPI_FLOAT,          sizeof(float),          false, true, true)},
		{ MPI_DOUBLE,         Datatype(MPI_DOUBLE,         sizeof(double),         false, true, true)},
		{ MPI_LONG_LONG_INT,  Datatype(MPI_LONG_LONG_INT,  sizeof(long long int),  false, true, true)},
		{ MPI_LONG_LONG,      Datatype(MPI_LONG_LONG,      sizeof(long long),      false, true, true)},
		{ MPI_FLOAT_INT,		Datatype(MPI_FLOAT_INT,		 sizeof(float_int_type), false, false, false)},
		{ MPI_LONG_INT,		Datatype(MPI_LONG_INT,		 sizeof(long_int_type),  false, false, false)},
		{ MPI_DOUBLE_INT,		Datatype(MPI_DOUBLE_INT,	 sizeof(double_int_type),false, false, false)},
		{ MPI_2INT,		    Datatype(MPI_2INT,	 		 sizeof(int_int_type),   false, false, false)},
#if 0
		{ MPI_LONG_DOUBLE, Datatype(MPI_LONG_DOUBLE,    sizeof(long double),    false, true, true)},
#endif
	};

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

int Universe::save(std::ostream &t)
{
	// save derived datatypes
	// todo deferred

	// save groups
	int group_size = groups.size();
	t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
	for (auto &g : groups)
	{
		int value = g->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = g->get_process_list().size();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		for (auto p : g->get_process_list())
		{
			t.write(reinterpret_cast<char *>(&p), sizeof(int));
		}
	}

	// save communicators
	int comm_size = communicators.size();
	t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
	for(auto &c : communicators)
	{
		int value = c->get_rank();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_pt2pt();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_coll();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		bool intra = c->get_is_intra();
		t.write(reinterpret_cast<char *>(&intra), sizeof(bool));
		value = c->get_local_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_remote_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
	}

	// save composites
	int err = MPI_SUCCESS;
	//err = progress->save(t);

	return err;
}

int Universe::load(std::istream &t)
{
	// todo mpi stages load appropriate engine
	progress = std::make_unique<BlockingProgress>();

	debug("universe is loading");
	return MPI_SUCCESS;
}

void Universe::halt()
{
	Daemon &daemon = Daemon::get_instance();
	int err = daemon.send_clean_up();
	// todo handle error

	// propagate halt
	progress->halt();

	// destroy progress
	Progress *engine = progress.get();
	progress.release();

	delete engine;

	progress = std::make_unique<BlockingProgress>();
}

}
