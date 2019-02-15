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
	;
}

Header *TCPTransport::ordered_recv()
{
	;
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
