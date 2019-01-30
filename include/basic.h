#ifndef __EXAMPI_BASIC_H
#define __EXAMPI_BASIC_H
/*

   Exampi public "mpi.h"  V 0.0

   Authors: Shane Matthew Farmer, Nawrin Sultana, Anthony Skjellum

*/

#include <global.h>
#include <UDPSocket.h>
#include <config.h>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <sys/uio.h>

namespace exampi
{

const std::string runtimeConfig("mpihosts.stdin.tmp");

class Buf : public BufAbstract
{
private:
	struct iovec v;

public:
	Buf(void *p, size_t sz)
	{
		v.iov_base = p;
		v.iov_len = sz;
	}

	struct iovec iov()
	{
		return v;
	}
};


} // namespace exampi

#endif //guard
