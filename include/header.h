#ifndef __EXAMPI_HEADER_H
#define __EXAMPI_HEADER_H

#include "protocol.h"
#include "envelope.h"

namespace exampi
{

struct Header
{
	Protocol protocol;
	Envelope envelope;
};

typedef std::unique_ptr<Header, std::function<void(Header *)>> Header_uptr;

}

#endif
