#ifndef __EXAMPI_TCPTRANSPORT_H
#define __EXAMPI_TCPTRANSPORT_H

#include "abstract/transport.h"

namespace exampi
{

class TCPTransport: public Transport
{
private:

public:
	TCPTransport();
	~TCPTransport();

	const std::map<Protocol, size_t> &provided_protocols() const;

	Header_uptr ordered_recv();

	void fill(Header_uptr, Request *);

	void reliable_send(const Protocol, const Request *);

	int save(std::ostream &);
	int load(std::istream &);
	int halt();
};

}

#endif
