#ifndef __EXAMPI_DECIDER_H
#define __EXAMPI_DECIDER_H

#include "protocol.h"

namespace exampi
{

struct Decider
{
	Protocol decide(const Request_ptr request, const Universe &universe) const = 0;
};

}

#endif
