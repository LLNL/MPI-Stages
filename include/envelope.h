#ifndef __EXAMPI_ENVELOPE_H
#define __EXAMPI_ENVELOPE_H

#include "mpi.h"

namespace exampi
{

struct Envelope
{
	int epoch;
	int context;
	int source;
	int destination;
	int tag;
};

}

#endif
