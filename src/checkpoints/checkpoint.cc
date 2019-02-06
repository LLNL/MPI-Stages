#include "checkpoints/checkpoint.h"

#include <basic.h>
#include "engines/progress.h"
#include "universe.h"

namespace exampi
{

void BasicCheckpoint::save()
{
	Universe& universe = Universe::get_root_universe();
	
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

	// TODO
	//universe.progress->save(target);
	//universe.transport->save(target);
	//universe.interface->save(target);

	long long int end = target.tellp();
	size = end - begin;
	target.clear();
	target.seekp(0, std::ofstream::beg);

	target.write(reinterpret_cast<char *>(&size), sizeof(long long int));
	target.close();

	if (universe.errhandler->isErrSet() != 1)
	{
		// needed in case process dies
		// XXX could also send to daemon
		// write out epoch number
		universe.epoch++;
		std::ofstream ef(universe.epoch_config);
		ef << universe.epoch;
		ef.close();
	}
}

int BasicCheckpoint::load()
{
	Universe& universe = Universe::get_root_universe();

	if(universe.epoch == 0) // first init
	{
		// TODO we are getting rid of init
		//universe.transport->init();
		//universe.progress->init();

		return MPI_SUCCESS;
	}
	else   // subsequent init
	{
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

		Daemon& daemon = Daemon::get_instance();
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
