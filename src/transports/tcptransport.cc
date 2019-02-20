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

Header_uptr TCPTransport::ordered_recv()
{
	throw;
}

void TCPTransport::fill(Header_uptr header, Request *request)
{
	;
}

void TCPTransport::reliable_send(const Protocol, const Request *request)
{
	;
}

}
