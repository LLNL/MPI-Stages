#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <limits>

#include "transports/udptransport.h"
#include "universe.h"
#include "config.h"

namespace exampi
{

UDPTransport::UDPTransport() : header_pool(32), payload_pool(32)
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
	{
		debug("ERROR: socket creation failed: " << socket_recv);

		throw UDPTransportCreationException();
	}

	// setsockopt to reuse address/port
	int opt = 1;
	setsockopt(socket_recv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	           sizeof(opt));

	// garuntees no local collision
	Universe &universe = Universe::get_root_universe();
	int port = std::stoi(std::string(std::getenv("EXAMPI_UDP_TRANSPORT_BASE"))) +
	           universe.rank;
	debug("udp transport port: " << port);

	// bind locally
	struct sockaddr_in address_local;
	address_local.sin_family = AF_INET;
	address_local.sin_addr.s_addr = INADDR_ANY;
	address_local.sin_port = htons(port);

	if(bind(socket_recv, (sockaddr *)&address_local, sizeof(address_local)) < 0)
	{
		debug("ERROR: socket binding failed");

		throw UDPTransportBindingException();
	}

	// prepare msg header
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;

	// cache remote addresses
	// todo static connection building, don't do this forever
	Config &config = Config::get_instance();

	for(long int rank = 0; rank < universe.world_size; ++rank)
	{
		std::string descriptor = config[std::to_string(rank)];
		size_t delimiter = descriptor.find_first_of(":");
		std::string ip = descriptor.substr(0, delimiter);
		int port = std::stoi(descriptor.substr(delimiter+1));

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);

		cache.insert({rank, addr});
	}

	// setting available protocols for UDPTransport
	available_protocols[Protocol::EAGER]		= 65507 - sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= 65507 - sizeof(Header);

	available_protocols[Protocol::SEQ]			= std::numeric_limits<size_t>::max() - sizeof(Header);
	available_protocols[Protocol::SEQ_ACK]		= std::numeric_limits<size_t>::max() - sizeof(Header);
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

Header *UDPTransport::ordered_recv()
{
	// early exit test
	char test;
	ssize_t size = recv(socket_recv, &test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
	if(size <= 0)
	{
		return nullptr;
	}

	// commit to do receive operation
	std::lock_guard<std::mutex> lock(guard);

	// todo is it possible to reduce this to less recvmsgs?
	//      - not if we want to have variable length messages
	//      - other solution is fixed size messages, always needs to send/recv fixed size

	debug("receive from underlying socket available");
	Header *header = header_pool.allocate();

	iovec iovs[3];
	iovs[0].iov_base = header;
	iovs[0].iov_len = sizeof(Header);

	int payload_length;
	iovs[1].iov_base = &payload_length;
	iovs[1].iov_len = sizeof(int);

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 2;
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	// peek receive and gain information if available
	// if not then exit gracefully
	int err = recvmsg(socket_recv, &hdr, MSG_PEEK | MSG_DONTWAIT);
	if(err <= 0)
	{
		header_pool.deallocate(header);

		debug("false receive operation, another thread stole work");
		return nullptr;
	}
	else
	{
		debug("received header + size of size " << err);
		debug("header: e " << header->envelope.epoch <<
		      			" c " << header->envelope.context <<
		      			" s " << header->envelope.source <<
		      			" d " << header->envelope.destination <<
		      			" t " << header->envelope.tag);
		debug("payload length " << payload_length);

		UDPTransportPayload *payload = nullptr;
		if(payload_length > 0)
		{
			payload = payload_pool.allocate();

			// prepare receive payload
			iovs[2].iov_base = payload;
			iovs[2].iov_len = payload_length;

			hdr.msg_iovlen = 3;
		}

		// properly receive the message from the kernel
		// todo this is really copying it twice, how can we avoid that?
		//      can optimize, partial reading in of header + payload - only copy once
		int err = recvmsg(socket_recv, &hdr, 0);
		if(err <= 0)
		{
			if(payload_length > 0)
				payload_pool.deallocate(payload);

			debug("throwing UDPTransportPayloadReceiveError");
			throw UDPTransportPayloadReceiveError();
		}
		else
		{
			debug("received payload of size " << err);
		}

		// store the data for later fill
		if(payload_length > 0)
		{
			debug("payload received: " << ((int*)payload)[0]);
			payload_buffer[(const Header *)header] = payload;
		}
	}

	return header;
}

void UDPTransport::fill(const Header *header, Request *request)
{
	// look up payload with respect to header
	UDPTransportPayload *payload = payload_buffer[header];

	void *err = std::memcpy((void *)request->payload.buffer, payload, sizeof(int));
	if(err == nullptr)
	{
		throw UDPTransportFillError();
	}
	else
	{
		payload_pool.deallocate(payload);
	}
}

void UDPTransport::reliable_send(const Protocol protocol,
                                 const Request *request)
{
	std::lock_guard<std::mutex> lock(guard);

	// todo depends on datatype
	//      we currently only support vector/block
	iovec iovs[4];

	// protocol
	iovs[0].iov_base = (void *)&protocol;
	iovs[0].iov_len = sizeof(Protocol);

	// request->envelope
	iovs[1].iov_base = (void *)&request->envelope;
	iovs[1].iov_len = sizeof(Envelope);

	debug("envelope to send: e " << request->envelope.epoch <<
	      " c " << request->envelope.context <<
	      " s " << request->envelope.source <<
	      " d " << request->envelope.destination <<
	      " t " << request->envelope.tag);

	debug("sending payload " << ((int*)request->payload.buffer)[0]);

	// payload length
	// todo datatype packing, gather
	int payload_size = request->payload.count * sizeof(int);
	iovs[2].iov_base = (void *)&payload_size;
	iovs[2].iov_len = sizeof(int);

	// request->buffer;
	iovs[3].iov_base = (void *)request->payload.buffer;
	iovs[3].iov_len = sizeof(int) * request->payload.count;

	// todo rank -> root commmunicator -> address
	sockaddr_in &addr = cache[request->envelope.destination];

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 4;
	hdr.msg_name = &addr;
	hdr.msg_namelen = sizeof(addr);

	int err = sendmsg(socket_recv, &hdr, 0);
	if(err <= 0)
	{
		throw UDPTransportSendError();
	}
	else
	{
		debug("sent " << err << " bytes");
	}
}

const std::map<Protocol, size_t> &UDPTransport::provided_protocols() const
{
	return available_protocols;
}

int UDPTransport::save(std::ostream &r)
{
	return MPI_SUCCESS;
}

int UDPTransport::load(std::istream &r)
{
	return MPI_SUCCESS;
}

int UDPTransport::halt()
{
	// todo mpi stages no idea what this does, from original udp transport
//	//std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
//	char buffer[2];
//	struct sockaddr_storage src_addr;
//
//	struct iovec iov[1];
//	iov[0].iov_base=buffer;
//	iov[0].iov_len=sizeof(buffer);
//
//	struct msghdr message;
//	message.msg_name=&src_addr;
//	message.msg_namelen=sizeof(src_addr);
//	message.msg_iov=iov;
//	message.msg_iovlen=1;
//	message.msg_control=0;
//	message.msg_controllen=0;
//
//
//	//std::cout << debug() <<
//	//          "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;
//
//	//std::cout << debug() << "basic::Transport::udp::recv\n";
//
//	recvmsg(recvSocket.getFd(), &message, MSG_WAITALL);
//	//std::cout << debug() << "basic::Transport::udp::recv exiting\n";
//	//std::cout << debug() << "basic::Transport::receive returning" << std::endl;
	return MPI_SUCCESS;
}

}