#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include <basic.h>
#include "engines/progress.h"

namespace exampi
{
namespace basic
{

class Checkpoint : public exampi::i::Checkpoint
{
public:
	virtual void save()
	{
		// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
		std::stringstream filename;
		filename << exampi::global::epoch << "." << exampi::global::rank << ".cp";
		std::ofstream target(filename.str(), std::ofstream::out);

		long long int size = 0;
		long long int begin = target.tellp();
		target.write(reinterpret_cast<char *>(&size), sizeof(long long int));

		// save the global datatype map
		//uint32_t typecount = exampi::global::datatypes.size();
		//target.write(reinterpret_cast<char *>(&typecount), sizeof(uint32_t));
		for(auto i : exampi::global::datatypes)
		{
			//i.save(target);
		}

		exampi::global::progress->save(target);
		exampi::global::transport->save(target);
		//exampi::global::interface->save(target);
		long long int end = target.tellp();
		size = end - begin;
		target.clear();
		target.seekp(0, std::ofstream::beg);

		target.write(reinterpret_cast<char *>(&size), sizeof(long long int));
		target.close();

		if (exampi::global::handler->isErrSet() != 1)
		{
			exampi::global::epoch++;
			std::ofstream ef(exampi::global::epochConfig);
			ef << exampi::global::epoch;
			ef.close();
		}


	}

	virtual int load()
	{
		if(exampi::global::epoch == 0) // first init
		{
			exampi::global::transport->init();
			exampi::global::progress->init();
			return MPI_SUCCESS;
		}
		else   // subsequent init
		{
			// get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
			std::stringstream filename;
			filename << exampi::global::epoch - 1 << "." << exampi::global::rank << ".cp";
			std::ifstream target(filename.str(), std::ifstream::in);

			long long int pos;
			target.read(reinterpret_cast<char *>(&pos), sizeof(long long int));

			// save the global datatype map
			//target.write(&typecount, sizeof(uint32_t));
			for(auto i : exampi::global::datatypes)
			{
				// i.save(target);
			}
			exampi::global::progress->load(target);
			exampi::global::transport->load(target);
			exampi::global::progress->barrier();
			//exampi::global::interface->save(target);
			target.close();
			std::ifstream ef(exampi::global::epochConfig);
			ef >> exampi::global::epoch;
			ef.close();
		}
		return MPIX_SUCCESS_RESTART;
	}
};

}
} // exampi::basic::

#endif
