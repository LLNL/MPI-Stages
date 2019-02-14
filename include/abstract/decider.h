#ifndef __EXAMPI_DECIDER_H
#define __EXAMPI_DECIDER_H

#include "protocol.h"
#include "universe.h"
#include "request.h"

namespace exampi
{

struct Decider
{
	virtual Protocol decide(const Request_ptr request, const Universe &universe) const = 0;
};

}

#endif
