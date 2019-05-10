// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>

#include "transports/tcptransport.h"
#include "universe.h"
#include "configuration.h"

namespace exampi
{

TCPTransport::TCPTransport() :
	header_pool(32)
{
	// set available protocols
	available_protocols[Protocol::EAGER]		= std::numeric_limits<size_t>::max() -
	        sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= std::numeric_limits<size_t>::max() -
	        sizeof(Header);

	// create socket
	debug("creating server_socket socket");
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket == 0)
	{
		throw TCPTransportSocketCreationFailed();
	}

	// set non-blocking server socket
	//int flags;
	//if((flags = fcntl(server_socket, F_GETFL, 0)) < 0)
	//{
	//	throw TCPTransportNonBlockError();
	//}
	//if(fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) < 0)
	//{
	//	throw TCPTransportNonBlockError();
	//}

	// bind socket
	Universe &universe = Universe::get_root_universe();
	int port = std::stoi(std::string(std::getenv("EXAMPI_TCP_TRANSPORT_BASE"))) +
	           universe.rank;
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

	// static triangular connection building
	debug("build tcp connections statically");
	for(int rank = 0; rank < universe.rank; ++rank)
	{
		debug("static connection building to world_rank" << rank);
		connections[rank] = rank_connect(rank);

		// write into fds
		struct pollfd pfd;
		pfd.fd = connections[rank];
		pfd.events = POLLIN;
		pfd.revents = 0;

		fds.push_back(pfd);
	}
	for(int rank = universe.rank+1; rank < universe.world_size; ++rank)
	{
		debug("accepting connection from rank idx " << rank);

		sockaddr_in addr;
		unsigned int addrlen = sizeof(addr);
		int client = accept4(server_socket, (sockaddr *)&addr, (socklen_t *)&addrlen,
		                     SOCK_NONBLOCK);

		// recv rank
		// ordering is not garunteed, especially with dynamic connections
		int rank_recv = -1;

		while(rank_recv == -1)
		{
			int err = recv(client, &rank_recv, sizeof(rank_recv), MSG_WAITALL);
			if(err < 0)
			{
				//throw TCPTransportConnectionFailed();
				usleep(10 * 1000); // 10ms
			}
			else
			{
				debug("received rank " << rank_recv);
			}
		}

		// insert into connections
		connections[rank_recv] = client;

		struct pollfd pfd;
		pfd.fd = connections[rank];
		pfd.events = POLLIN;
		pfd.revents = 0;

		fds.push_back(pfd);
	}
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
	int client = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if(client == 0)
	{
		debug("ERROR: failed to create socket for client communication.");
		throw TCPTransportSocketCreationFailed();
	}

	Configuration &config = Configuration::get_instance();

	// world_rank -> sockaddr
	//std::string descriptor = config[std::to_string(world_rank)];
	//size_t delimiter = descriptor.find_first_of(":");
	//std::string ip = descriptor.substr(0, delimiter);
	//int port = std::stoi(descriptor.substr(delimiter+1));

	const auto &descriptor = config[std::to_string(world_rank)];

	//std::string address = descriptor<std::string>["address"];
	//std::string address = descriptor.get<std::string>("address");

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(
	                           descriptor["address"].get<std::string>().c_str());
	//addr.sin_addr.s_addr = inet_addr(address.c_str());
	addr.sin_port = htons(descriptor["tcp_port"].get<int>());

	// connect to rank tcp server_socket
	bool connected = false;
	while(!connected)
	{
		int err = connect(client, (sockaddr *)&addr, sizeof(addr));
		if(err < 0)
		{
			//throw TCPTransportConnectionFailed();
			usleep(100000);
		}
		else
		{
			connected = true;
		}
	}

	// send rank
	Universe &universe = Universe::get_root_universe();
	int err = send(client, &universe.rank, sizeof(int), 0);
	if(err < 0)
	{
		throw TCPTransportConnectionFailed();
	}

	return client;
}

Header_uptr TCPTransport::iterate()
{
	// arrivals > 0
	if((arrivals - peeked) == 0)
	{
		debug("false iterate call, no arrivals");
		return nullptr;
	}

	// iterate for first one
	for(auto &pollfd: fds)
	{
		if(pollfd.revents != 0)
		{
			// if server_socket then accept connection
			if(pollfd.fd == server_socket)
			{
				debug("found server socket with poll");
				// todo dynamic connection building
			}

			// otherwise receive message
			else
			{
				Header *header = header_pool.allocate();

				// set up iovs
				iovec iovs[2];

				// protocol field
				iovs[0].iov_base = (void *)&header->protocol;
				iovs[0].iov_len = sizeof(Protocol);

				// envelope field
				iovs[1].iov_base = (void *)&header->envelope;
				iovs[1].iov_len = sizeof(Envelope);

				// fill msghdr
				hdr.msg_iov = iovs;
				hdr.msg_iovlen = 2;

				int err = recvmsg(pollfd.fd, &hdr, MSG_PEEK);
				if(err < 0)
				{
					throw TCPTransportPeekError();
				}
				else
				{
					debug("peeked message header from rank " << header->envelope.source);

					// only process first socket
					pollfd.revents = 0;
					peeked += 1;

					return Header_uptr(header, [this] (Header *header) -> void { this->header_pool.deallocate(header); });
				}
			}

		}
	}

	return nullptr;
}

Header_uptr TCPTransport::ordered_recv()
{
	std::lock_guard<std::mutex> lock(guard);

	// check for previously unprocessed arrivals
	if((arrivals - peeked) > 0)
	{
		// iterate fds check for first arrival
		debug("found arrivals already present");
		return iterate();
	}

	// if no known messages
	else if(arrivals == 0)
	{
		// poll all sockets, returning immediately
		int err = poll(fds.data(), fds.size(), 0);
		if(err < 0)
		{
			throw TCPTransportPollError();
		}
		else if(err > 0)
		{
			debug("found arrivals " << err);
			arrivals = err;
			return iterate();
		}
		else
		{
			return nullptr;
		}
	}
	else
		return nullptr;
}

void TCPTransport::fill(Header_uptr header, Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// look up file descriptor for source
	int world_rank = header->envelope.source;
	int fd = connections[world_rank];

	// set up iovs
	iovec iovs[4];

	// protocol field
	iovs[0].iov_base = (void *)&header->protocol;
	iovs[0].iov_len = sizeof(Protocol);

	// envelope field
	iovs[1].iov_base = (void *)&header->envelope;
	iovs[1].iov_len = sizeof(Envelope);

	// message size field
	//size_t payload_size;
	//iovs[2].iov_base = (void*)&payload_size;
	//iovs[2].iov_len = sizeof(size_t);

	// message data
	iovs[2].iov_base = (void *)request->payload.buffer;
	iovs[2].iov_len = request->payload.count *
	                  request->payload.datatype->get_extent();
	// todo how to deal with underwriting/overwriting? valid MPI

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 3;

	// read msghr from stream
	int err = recvmsg(fd, &hdr, 0);
	if(err < 0)
	{
		throw TCPTransportFillError();
	}
	else
	{
		debug("filled message from rank " << header->envelope.source << " request " <<
		      request);
	}

	// finalize message arrival
	peeked -= 1;
	arrivals -= 1;
}

void TCPTransport::reliable_send(const Protocol protocol,
                                 const Request *request)
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
		// dynamic connection building
		debug("rank " << request->envelope.destination << " is not connected");
		client = rank_connect(world_rank);
	}

	// prepare iov send hdr
	debug("filling iovs with tcp message");
	iovec iovs[4];

	// fill protocol field
	iovs[0].iov_base = (void *)&protocol;
	iovs[0].iov_len = sizeof(Protocol);

	// fill envelope field
	iovs[1].iov_base = (void *)&request->envelope;
	iovs[1].iov_len = sizeof(Envelope);

	// fill message size field
	//size_t payload_size = request->payload.count * request->payload.datatype->get_extent();
	//iovs[2].iov_base = (void*)&payload_size;
	//iovs[2].iov_len = sizeof(size_t);

	// fill message data
	iovs[2].iov_base = (void *)request->payload.buffer;
	iovs[2].iov_len = request->payload.count *
	                  request->payload.datatype->get_extent();

	// fill msghdr
	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 3;

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

int TCPTransport::save(std::ostream &t)
{
	return MPI_SUCCESS;
}

int TCPTransport::load(std::istream &t)
{
	return MPI_SUCCESS;
}

int TCPTransport::cleanup()
{
	return MPI_SUCCESS;
}
int TCPTransport::halt()
{
	return MPI_SUCCESS;
}

}
