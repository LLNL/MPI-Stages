#include "transports/tcptransport.h"

namespace exampi
{

TCPTransport::TCPTransport()
{
	;
}

TCPTransport::~TCPTransport()
{
	;
}

const std::map<Protocol, size_t> &TCPTransport::provided_protocols() const
{
	return std::map<Protocol, size_t>();
}

Header *TCPTransport::ordered_recv()
{
	return nullptr;
}

void TCPTransport::fill(const Header *header, Request *request)
{
	;
}

void TCPTransport::reliable_send(const Protocol, const Request *request)
{
	;
}

}
