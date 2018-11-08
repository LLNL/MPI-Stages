#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include <basic.h>
#include "engines/progress.h"

namespace exampi
{

class BasicCheckpoint: public Checkpoint
{
public:
	BasicCheckpoint(); 

	void save();
	int load();
};

} // exampi

#endif
