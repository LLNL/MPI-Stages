#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include <list>

#include "abstract/matcher.h"

namespace exampi
{

class SimpleMatcher final: public Matcher
{
private:
	std::list<Request> posted_receive_queue;
	std::list<Request> unexpected_message_queue;

public:
	int	post(Request *request);

	int match(Request *request);

	int progress();
};

}

#endif 
