#include "transports/tcptransport.h"

namespace exampi
{

TCPTransport::TCPTransport()
{
	// set available protocols
	available_protocols[Protocol::EAGER]		= 1024 - sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= 1024 - sizeof(Header);
	
	// create socket
	
	// bind socket

	// listen on socket

	// prepare header
}

TCPTransport::~TCPTransport()
{
	// close all client connections

	// close server socket
}

const std::map<Protocol, size_t> &TCPTransport::provided_protocols() const
{
	return available_protocols;
}

Header *TCPTransport::ordered_recv()
{
	std::lock_guard<std::mutex> lock(guard);

	return nullptr;

	// check for connection requests

	// peek for message header	
	
	// if no data read it fully
}

void TCPTransport::fill(const Header *header, Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// read from tcp socket the associated data	
}

void TCPTransport::reliable_send(const Protocol, const Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// check for connection requests

	// if not connected, initiate connection

	// send msg across connection	
}

}
