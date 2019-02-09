#include "universe.h"

#include "checkpoints/checkpoint.h"

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

	rank = std::stoi(std::string(std::getenv("EXAMPI_RANK")));
	epoch_config = std::string(std::getenv("EXAMPI_EPOCH_FILE"));
	epoch = std::stoi(std::string(std::getenv("EXAMPI_EPOCH")));
	world_size = std::stoi(std::string(std::getenv("EXAMPI_WORLD_SIZE")));	
	
	debug("creating checkpoint");
	checkpoint = new BasicCheckpoint();

	// MPI WORLD GROUP
	debug("generating world group");
	std::list<int> rankList;
	for(int idx = 0; idx < world_size; ++idx)
		rankList.push_back(idx);
	world_group.set_process_list(rankList);

	groups.push_back(&world_group);

	// MPI_COMM_WORLD
	debug("generating world communicator");
	world_comm.is_intra = true;
	world_comm.local = &world_group;
	world_comm.remote = &world_group;
	world_comm.set_rank(rank);
	world_comm.set_context(0, 1);

	communicators.push_back(&world_comm);

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


	bool first = true;
	for(auto &&com : communicators)
	{
		if(first)
		{
			first = false;
			continue;
		}
        delete com;
	}
    communicators.clear();

	first = true;
	debug("deleting all groups");   
    // delete groups
	for (auto &&group : groups)
    {
		if(first)
		{
			first = false;
			continue;
		}

        delete group;
    }
    groups.clear();

	delete progress;
	delete checkpoint;

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

}
