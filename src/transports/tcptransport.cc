#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>

#include "transports/tcptransport.h"
#include "universe.h"
#include "config.h"

namespace exampi
{

TCPTransport::TCPTransport() :
	arrivals(0)
{
	// set available protocols
	available_protocols[Protocol::EAGER]		= std::numeric_limits<size_t>::max() - sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= std::numeric_limits<size_t>::max() - sizeof(Header);
	
	// create socket
	debug("creating server_socket socket");
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket == 0)
	{
		throw TCPTransportSocketCreationFailed();
	}

	// set non-blocking
	int flags;
	if((flags = fcntl(server_socket, F_GETFL, 0)) < 0) 
	{
		throw TCPTransportNonBlockError();
	} 
	if(fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) < 0) 
	{ 
		throw TCPTransportNonBlockError();
	} 
	
	// bind socket
	Universe &universe = Universe::get_root_universe();
	int port = std::stoi(std::string(std::getenv("EXAMPI_TCP_TRANSPORT_BASE"))) + universe.rank;
	debug("tcp transport port: " << port);

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(port);
	
	if(bind(server_socket, (sockaddr *)&local, sizeof(local)) < 0)
	{
		throw TCPTransportBindError();
	}

	// listen on socket
	if(listen(server_socket, 5) < 0)
	{
		throw TCPTransportListenError();
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

	// close server_socket
	debug("closing server_socket socket");
	close(server_socket);
}

const std::map<Protocol, size_t> &TCPTransport::provided_protocols() const
{
	return available_protocols;
}

int TCPTransport::rank_connect(int world_rank)
{
	int client = socket(AF_INET, SOCK_STREAM, 0);
	if(client == 0)
	{
		debug("ERROR: failed to create socket for client communication.");
		throw TCPTransportSocketCreationFailed();
	}
	
	Config &config = Config::get_instance();

	// world_rank -> sockaddr
	std::string descriptor = config[std::to_string(world_rank)];
	size_t delimiter = descriptor.find_first_of(":");
	std::string ip = descriptor.substr(0, delimiter);
	int port = std::stoi(descriptor.substr(delimiter+1));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	// connect to rank tcp server_socket
	// todo can fail to connect, retry after delay...
	int err = connect(client, (sockaddr *)&addr, sizeof(addr));
	if(err < 0)
	{
		throw TCPTransportConnectionFailed();
	}

	return client;
}

Header_uptr TCPTransport::iterate()
{
	// arrivals > 0

	// iterate for first one
	
		// if server_socket then accept connection

		// else return header

	return nullptr;
	// find first fd
	//for(auto)
	//{
	//	
	//}
	// TODO

	// reduce arrivals

	// read in header

	// TODO move to iterate as special socket fd
	//// check for connection on server_socket
	//// todo could use SIGIO as handler
	//struct sockaddr_in addr;
	//int client = accept(server_socket, &addr, sizeof(addr));
	//if(client < 0)
	//{
	//	// expected result, non-blocking socket
	//	// todo should check errno
	//	//      throw if not EAGAIN || EWOULDBLOCK
	//}
	//else
	//{
	//	// insert client into connections
	//	// TODO
	//	// both fds and connections
	//}
}

Header_uptr TCPTransport::ordered_recv()
{
	std::lock_guard<std::mutex> lock(guard);

	// check for previously unprocessed arrivals
	if(arrivals > 0)
	{
		// iterate fds check for first arrival
		return iterate();
	}
	
	// otherwise poll sockets
	else
	{
		// poll all sockets, returning immediately
		int err = poll(fds.data(), fds.size(), 0);
		if(err < 0)
		{
			throw TCPTransportPollError();
		}
		else if(err > 0)
		{
			arrivals = err;
			return iterate();
		}
		else
		{
			return nullptr;
		}
	}
}

void TCPTransport::fill(Header_uptr header, Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// read from tcp socket the associated data	
	// TODO straight forward, read from socket into request buffer(s)
	// no buffering required as in UDPTransport
}

void TCPTransport::reliable_send(const Protocol protocol, const Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// todo use communicator -> world_communicator translation
	//      communitor.group.get_root_rank(destination)
	int world_rank = request->envelope.destination;

	// check if connected
	debug("fetching connection");
	int &client = connections[world_rank];
	if(client == 0)
	{
		debug("rank " << request->envelope.destination << " is not connected");
		client = rank_connect(world_rank);
	}

	// prepare iov send hdr
	debug("filling iovs with tcp message");
	iovec iovs[4];

	// fill protocol field
	iovs[0].iov_base = (void*)&protocol;
	iovs[0].iov_len = sizeof(Protocol);
	
	// fill envelope field
	iovs[1].iov_base = (void*)&request->envelope;
	iovs[1].iov_len = sizeof(Envelope);
	
	// fill message size field
	size_t payload_size = request->payload.count * request->payload.datatype->get_extent();
	iovs[2].iov_base = (void*)&payload_size;
	iovs[2].iov_len = sizeof(size_t);

	// fill message data
	iovs[3].iov_base = (void*)request->payload.buffer; 
	iovs[3].iov_len = request->payload.count * request->payload.datatype->get_extent();
	
	// fill msghdr
	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 4;

	// send msg across wire	
	debug("sendmsg to rank " << request->envelope.destination);
	int err = sendmsg(client, &hdr, 0);
	if(err <= 0)
	{
		throw TCPTransportSendError();
	}
	else
	{
		debug("sent " << err << " bytes over tcptransport to world_rank " << 
		      world_rank << " : comm_rank " << request->envelope.destination);
	}
}

}
