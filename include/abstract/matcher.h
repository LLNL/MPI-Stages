#ifndef __EXAMPI_ABSTRACT_MATCHING_H
#define __EXAMPI_ABSTRACT_MATCHING_H

#include "request.h"
#include "header.h"
#include "abstract/stages.h"

namespace exampi
{

class Matcher: virtual public Stages
{
public:
	virtual void post_request(Request *) = 0;
	virtual void post_header(Header_uptr) = 0;

	virtual std::tuple<Header_uptr, Request *> progress() = 0;
};

}

#endif
