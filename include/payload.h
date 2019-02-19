#ifndef __EXAMPI_PAYLOAD_H
#define __EXAMPI_PAYLOAD_H

#include "datatype.h"

namespace exampi
{

struct Payload
{
	const void *buffer;
	MPI_Datatype datatype;
	int count;
};

}

#endif
