#ifndef __EXAMPI_SIMPLE_DECIDER_H
#define __EXAMPI_SIMPLE_DECIDER_H

#include "abstract/decider.h"

namespace exampi
{

class SimpleDecider: public Decider
{
	Protocol decide(const Request_ptr request, const Universe &universe) const;	
};

}

#endif
