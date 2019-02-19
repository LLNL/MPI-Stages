#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include "abstract/checkpoint.h"

namespace exampi
{

class BasicCheckpoint: public Checkpoint
{
public:
	void save();
	int load();
};

} // exampi

#endif
