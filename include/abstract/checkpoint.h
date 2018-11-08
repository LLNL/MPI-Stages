#ifndef __EXAMPI_ABSTRACT_CHECKPOINT_H
#define __EXAMPI_ABSTRACT_CHECKPOINT_H

#include <mpi.h>

namespace exampi
{

class Checkpoint
{
public:
	virtual void save() = 0;
	virtual int load() = 0;
};

} // exampi::i
#endif
