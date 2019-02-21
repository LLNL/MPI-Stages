#include <unistd.h>

#include "transports/tcptransport.h"

namespace exampi
{

TCPTransport::TCPTransport()
{
	// set available protocols
	available_protocols[Protocol::EAGER]		= std::numeric_limits<size_t>::max() - sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= std::numeric_limits<size_t>::max() - sizeof(Header);
	
	// create socket
	debug("creating server socket");
	server = socket(AF_INET, SOCK_STREAM, 0);
	// TODO handle error

	// TODO set sock opt
	// set non blocking
	
	// bind socket
	int err = bind(server, );
	if(err < 0)
	{
		// TODO handle error
		throw;
	}

	// listen on socket
	err = listen(server, 5);
	if(err < 0)
	{
		// TODO handle error
		throw;
	}

	// prepare header
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;
	
	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;
}

TCPTransport::~TCPTransport()
{
	// close all client connections
	debug("closing all client connections");
	for(auto& [rank, connection] : connections)
	{
		debug("closing connection to rank " << rank);
		close(connection);
	}

	// close server socket
	debug("closing server socket");
	close(server);
}

const std::map<Protocol, size_t> &TCPTransport::provided_protocols() const
{
	return available_protocols;
}

int TCPTransport::connect(int world_rank)
{
	int client = socket(AF_INET, SOCK_STREAM, 0);
	if(client == 0)
	{
		debug("ERROR: failed to create socket for client communication.");
		throw TCPTransportSocketCreationFailed();
	}
	
	// world_rank -> sockaddr
	

	// connect to rank tcp server
	int err = connect(client, );
	if(err < 0)
	{
		throw TCPTransportConnectionFailed();
	}

	return client;
}

Header_uptr TCPTransport::ordered_recv()
{
	std::lock_guard<std::mutex> lock(guard);

	return nullptr;

	// check for connection on server socket
		// when accepting put into connections

	// peek for message header	
	
	// if no data read it fully
}

void TCPTransport::fill(Header_uptr header, Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// read from tcp socket the associated data	
}

void TCPTransport::reliable_send(const Protocol protocol, const Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	int world_rank = request->envelope.destination;

	// check if connected
	debug("fetching connection");
	int &client = connections[world_rank];
	if(client == 0)
	{
		debug("rank " << request->envelope.destination << " is not connected");
		client = connect(world_rank);
	}

	// prepare iov send hdr
	debug("filling iovs with tcp message");
	iovec iovs[4];

	iovs[0].iov_base = (void*)&protocol;
	iovs[0].iov_len = sizeof(Protocol);
	
	iovs[1].iov_base = (void*)&request->envelope;
	iovs[1].iov_len = sizeof(Envelope);
	
	long int payload_size = request->payload.count * request->payload.datatype->get_extent();
	iovs[2].iov_base = (void*)&payload_size;
	iovs[2].iov_len = sizeof(long int);

	// TODO requires packing
	iovs[3].iov_base = (void*)request->payload.buffer; 
	// datatype size
	iovs[3].iov_len = request->payload.count * request->payload.datatype->get_extent();
	
	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 4;

	// send msg across connection	
	debug("sendmsg to rank " << request->envelope.destination);
	int err = sendmsg(client, &hdr, 0);
	if(err <= 0)
	{
		throw TCPTransportSendError();
	}
	else
	{
		debug("sent " << err << " bytes over tcptransport");
	}
}

}
