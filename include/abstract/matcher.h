#ifndef __EXAMPI_ABSTRACT_MATCHING_H
#define __EXAMPI_ABSTRACT_MATCHING_H

#include "request.h"

namespace exampi
{

class Matcher
{
public:
	virtual int post(Request *request) = 0;

	virtual int match(Request *request) = 0;

	virtual int progress();
};

}

#endif
