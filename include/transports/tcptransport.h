#ifndef __EXAMPI_TCPTRANSPORT_H
#define __EXAMPI_TCPTRANSPORT_H

#include "abstract/transport.h"

namespace exampi
{

class TCPTransport: public Transport
{
private:
	std::mutex guard;

	std::map<Protocol, size_t> available_protocols;

public:
	TCPTransport();
	~TCPTransport();

	virtual const std::map<Protocol, size_t> &provided_protocols() const = 0;

	virtual Header *ordered_recv() = 0;

	virtual void fill(const Header *, Request *) = 0;

	virtual void reliable_send(const Protocol, const Request *) = 0;

	// todo mpi stages
	virtual int save(std::ostream &);
	virtual int load(std::istream &);
	virtual int halt();
};

}

#endif
