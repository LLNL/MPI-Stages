#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include "abstract/matcher.h"

namespace exampi
{

class SimpleMatcher: public Matcher
{
public:
	int	match(Request *request);
};

}

#endif
