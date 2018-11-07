#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include <basic.h>
#include "engines/progress.h"

namespace exampi
{

void BasicCheckpoint::save()
{
	// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
	std::stringstream filename;
	filename << exampi::epoch << "." << exampi::rank << ".cp";
	std::ofstream target(filename.str(), std::ofstream::out);

	long long int size = 0;
	long long int begin = target.tellp();
	target.write(reinterpret_cast<char *>(&size), sizeof(long long int));

	// save the global datatype map
	//uint32_t typecount = exampi::datatypes.size();
	//target.write(reinterpret_cast<char *>(&typecount), sizeof(uint32_t));
	for(auto i : exampi::datatypes)
	{
		//i.save(target);
	}

	exampi::progress->save(target);
	exampi::transport->save(target);
	//exampi::interface->save(target);
	long long int end = target.tellp();
	size = end - begin;
	target.clear();
	target.seekp(0, std::ofstream::beg);

	target.write(reinterpret_cast<char *>(&size), sizeof(long long int));
	target.close();

	if (exampi::handler->isErrSet() != 1)
	{
		exampi::epoch++;
		std::ofstream ef(exampi::epochConfig);
		ef << exampi::epoch;
		ef.close();
	}
}

int BasicCheckpoint::load()
{
	if(exampi::epoch == 0) // first init
	{
		exampi::transport->init();
		exampi::progress->init();
		return MPI_SUCCESS;
	}
	else   // subsequent init
	{
		// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
		std::stringstream filename;
		filename << exampi::epoch - 1 << "." << exampi::rank << ".cp";
		std::ifstream target(filename.str(), std::ifstream::in);

		long long int pos;
		target.read(reinterpret_cast<char *>(&pos), sizeof(long long int));

		// save the global datatype map
		//target.write(&typecount, sizeof(uint32_t));
		for(auto i : exampi::datatypes)
		{
			// i.save(target);
		}
		exampi::progress->load(target);
		exampi::transport->load(target);
		exampi::progress->barrier();
		//exampi::interface->save(target);
		target.close();
		std::ifstream ef(exampi::epochConfig);
		ef >> exampi::epoch;
		ef.close();
	}
	return MPIX_SUCCESS_RESTART;
}

} // exampi

#endif
