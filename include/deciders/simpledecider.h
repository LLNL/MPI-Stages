#ifndef __EXAMPI_SIMPLE_DECIDER_H
#define __EXAMPI_SIMPLE_DECIDER_H

#include <exception>

#include "abstract/decider.h"

namespace exampi
{

class SimpleDeciderUnknownException: public std::exception
{
	const char* what() const noexcept override
	{
		return "Operation is not recognized.";
	}
};

class SimpleDeciderReceiveDecisionException: public std::exception
{
	const char* what() const noexcept override
	{
		return "Receive operation is not implemented.";
	}	
};

class SimpleDecider: public Decider
{
	Protocol decide(const Request_ptr, const Universe &) const;
};

}

#endif
