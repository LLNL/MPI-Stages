#include <fstream>
#include <sstream>
#include <memory>

#include "checkpoints/checkpoint.h"
#include "engines/blockingprogress.h"
#include "universe.h"
#include "daemon.h"
#include "faulthandler.h"

namespace exampi
{

void BasicCheckpoint::save()
{
	Universe &universe = Universe::get_root_universe();

	// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
	std::stringstream filename;

	filename << universe.epoch << "." << universe.rank << ".cp";

	std::ofstream target(filename.str(), std::ofstream::out);

	long long int size = 0;
	long long int begin = target.tellp();
	target.write(reinterpret_cast<char *>(&size), sizeof(long long int));

	// save the global datatype map
	//uint32_t typecount = universe.datatypes.size();
	//target.write(reinterpret_cast<char *>(&typecount), sizeof(uint32_t));
	for(auto i : universe.datatypes)
	{
		//i.save(target);
	}

	// this saved groups/communicators
	//universe.progress->save(target);
	//universe.transport->save(target);
	//universe.interface->save(target);

	// save universe
	universe.save(target);

	long long int end = target.tellp();
	size = end - begin;
	target.clear();
	target.seekp(0, std::ofstream::beg);

	target.write(reinterpret_cast<char *>(&size), sizeof(long long int));
	target.close();

	FaultHandler &faulthandler = FaultHandler::get_instance();
	if(faulthandler.isErrSet() != 1)
	{
		// NOTE needed in case process dies
		// NOTE could also send to daemon

		// write out epoch number
		universe.epoch++;
		std::ofstream ef(universe.epoch_config);
		ef << universe.epoch;
		ef.close();
	}
}

int BasicCheckpoint::load()
{
	debug("fetching root universe for checkpointing");
	Universe &universe = Universe::get_root_universe();

	// first time executing
	if(universe.epoch == 0)
	{
		debug("epoch 0, starting BlockingProgress");

		// todo is there any other?
		universe.progress = std::make_unique<BlockingProgress>();

		return MPI_SUCCESS;
	}

	// after failure restart
	else
	{
		debug("epoch " << universe.epoch << ", reading checkpoint");

		// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
		std::stringstream filename;
		filename << universe.epoch - 1 << "." << universe.rank << ".cp";
		std::ifstream target(filename.str(), std::ifstream::in);

		long long int pos;
		target.read(reinterpret_cast<char *>(&pos), sizeof(long long int));

		// save the global datatype map
		//target.write(&typecount, sizeof(uint32_t));
		for(auto i : universe.datatypes)
		{
			// i.save(target);
		}

		//universe.progress->load(target);
		//universe.transport->load(target);

		universe.load(target);

		Daemon &daemon = Daemon::get_instance();
		daemon.barrier();

		//universe.interface->save(target);

		// read in epoch number
		target.close();
		std::ifstream ef(universe.epoch_config);
		ef >> universe.epoch;
		ef.close();
	}

	return MPIX_SUCCESS_RESTART;
}

} // exampi
