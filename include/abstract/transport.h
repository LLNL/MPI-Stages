#ifndef __EXAMPI_I_TRANSPORT_H
#define __EXAMPI_I_TRANSPORT_H

#include <mpi.h>
#include <protocol.h>

namespace exampi
{

class Transport
{
public:
	// remove these RAII
	virtual void init() = 0;
	virtual void finalize() = 0;

	// OLD
	//virtual std::future<int> send(std::vector<struct iovec> &iov, int dest,
	//                              MPI_Comm comm) = 0;
	//virtual std::future<int> receive(std::vector<struct iovec> &iov, MPI_Comm comm,
	//                                 ssize_t *count) = 0;
	//virtual int peek(std::vector<struct iovec> &iov, MPI_Comm comm) = 0;
	
	// stages
	virtual int save(std::ostream &r) = 0;
	virtual int load(std::istream &r) = 0;
	virtual int cleanUp(MPI_Comm comm) = 0;

	virtual std::unique_ptr<ProtocolMessage> absorb() = 0;
	virtual int send(std::unique_ptr<ProtocolMessage> message) = 0;
};

} // ::exampi

#endif
