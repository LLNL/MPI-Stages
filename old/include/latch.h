#ifndef __EXAMPI_I_LATCH_H
#define __EXAMPI_I_LATCH_H

#include <mpi.h>
#include <endpoint.h>

namespace exampi
{

class Latch
{
public:
	virtual void wait() = 0;
	virtual void signal() = 0;
};

} // ::exampi

#endif //  __EXAMPI_I_LATCH_H
