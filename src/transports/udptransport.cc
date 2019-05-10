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

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <limits>
#include <errno.h>

#include "transports/udptransport.h"
#include "universe.h"
#include "configuration.h"

namespace exampi
{

UDPTransport::UDPTransport() : header_pool(32), payload_pool(32)
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
	{
		debug("ERROR: socket creation failed: " << socket_recv);

		//throw UDPTransportCreationException();
		throw std::runtime_error("UDPTransport failed to create socket.");
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

		//throw UDPTransportBindingException();
		throw std::runtime_error("UDPTransport failed to bind socket.");
	}

	// prepare msg header
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;

	if (universe.epoch == 0)
		cache_endpoints();

	// setting available protocols for UDPTransport
	available_protocols[Protocol::EAGER]		= 65507 - sizeof(Header);
	available_protocols[Protocol::EAGER_ACK]	= 65507 - sizeof(Header);

	//cache_endpoints();

	//available_protocols[Protocol::SEQ]			= std::numeric_limits<size_t>::max() - sizeof(Header);
	//available_protocols[Protocol::SEQ_ACK]		= std::numeric_limits<size_t>::max() - sizeof(Header);
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

void UDPTransport::cache_endpoints()
{
	// cache remote addresses
	// todo static connection building, don't do this forever
	Universe &universe = Universe::get_root_universe();

	// todo static connection building, don't do this
	Configuration &config = Configuration::get_instance();

	for(long int rank = 0; rank < universe.world_size; ++rank)
	{
		// TODO remove now
//		std::string descriptor = config[std::to_string(rank)];
//		size_t delimiter = descriptor.find_first_of(":");
//
//		std::string ip = descriptor.substr(0, delimiter);
//		int port = std::stoi(descriptor.substr(delimiter+1));

		const auto &descriptor = config[std::to_string(rank)];

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(
		                           descriptor["address"].get<std::string>().c_str());
		addr.sin_port = htons(descriptor["udp_port"].get<int>());

		cache.insert({rank, addr});
	}
}

Header_uptr UDPTransport::ordered_recv()
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

	int payload_size;
	iovs[1].iov_base = &payload_size;
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
		debug("received (header + size) of size " << err);
		debug("header: e " << header->envelope.epoch <<
		      " c " << header->envelope.context <<
		      " s " << header->envelope.source <<
		      " d " << header->envelope.destination <<
		      " t " << header->envelope.tag);
		debug("payload length bytes " << payload_size);

		UDPTransportPayload *payload = nullptr;
		if(payload_size > 0)
		{
			debug("copying payload into buffer");

			payload = payload_pool.allocate();
			payload->payload_size = payload_size;

			// prepare receive payload
			iovs[2].iov_base = payload->payload;
			iovs[2].iov_len = payload_size;

			hdr.msg_iovlen = 3;
		}

		// properly receive the message from the kernel
		// todo this is really copying it twice, how can we avoid that?
		//      can optimize, partial reading in of header + payload - only copy once
		int err = recvmsg(socket_recv, &hdr, 0);
		if(err <= 0)
		{
			if(payload_size > 0)
				payload_pool.deallocate(payload);

			throw std::runtime_error("UDPTransport failed to receive payload.");
		}
		else
		{
			debug("received final payload of size " << err);
		}

		// store the data for later fill
		if(payload_size > 0)
		{
			payload_buffer[header] = payload;

			//TODO count == 0 fails debug("receive header " << header->envelope.source << " payload " << ((int*)payload->payload)[0]);
		}
	}

	return Header_uptr(header, [this] (Header *header) -> void { this->header_pool.deallocate(header); });
}

void UDPTransport::fill(Header_uptr header, Request *request)
{
	debug("udp transport filling " << request << " <- header " << header.get());

	// look up payload with respect to header
	UDPTransportPayload *payload = payload_buffer[header.get()];

	debug("header " << header->envelope.source << " payload " << ((
	            int *)payload->payload)[0]);
	debug("payload size " << payload->payload_size);

	// todo eventually we ask datatype to fill itself!
	void *err = std::memcpy((void *)request->payload.buffer, payload->payload,
	                        payload->payload_size);
	if(err == nullptr)
	{
		throw std::runtime_error("UDPTransport failed to fill.");
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

	// TODO add sequence number

	// todo depends on datatype
	//      we currently only support vector/block
	iovec iovs[4];

	// protocol
	iovs[0].iov_base = (void *)&protocol;
	iovs[0].iov_len = sizeof(Protocol);

	// request->envelope
	iovs[1].iov_base = (void *)&request->envelope;
	iovs[1].iov_len = sizeof(Envelope);

	debug("envelope to send: " <<
	      " e " << request->envelope.epoch <<
	      " c " << request->envelope.context <<
	      " s " << request->envelope.source <<
	      " d " << request->envelope.destination <<
	      " t " << request->envelope.tag);

	// payload length
	// TODO datatype packing, gather
	int payload_size = request->payload.count *
	                   request->payload.datatype->get_extent();
	iovs[2].iov_base = (void *)&payload_size;
	iovs[2].iov_len = sizeof(int);

	// request->buffer;
	iovs[3].iov_base = (void *)request->payload.buffer;
	iovs[3].iov_len = payload_size;

	debug("payload address " << request->payload.buffer);
	//TODO count == 0 fails debug("sent payload " << ((int*)request->payload.buffer)[0]);

	// todo rank -> root commmunicator -> address
	// note this currently works, because comm_dup is the only communicator construction allowed
	int world_rank = request->envelope.destination;
	sockaddr_in &addr = cache[world_rank];

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 4;
	hdr.msg_name = &addr;
	hdr.msg_namelen = sizeof(addr);

	// resend on error a couple times
	int bytes = -1;
	int cycles = 0;

	do
	{
		bytes = sendmsg(socket_recv, &hdr, 0);
		cycles++;

		if(bytes < 0)
		{
			debug("send failure in " << request->envelope.source << " " <<
			      request->envelope.destination);

			int errnum = errno;
			debug("error: " << strerror(errnum));

			debug("attempting resend");
		}
		else
		{
			debug("sent " << bytes << " bytes");
		}
	}
	// TODO tunable, resends 5 hardcoded
	while((bytes < 0) && cycles < 5);

	if((bytes < 0) && (cycles >= 5))
		throw std::runtime_error("UDPTransport failed to send after resends.");
}

const std::map<Protocol, size_t> &UDPTransport::provided_protocols() const
{
	return available_protocols;
}

int UDPTransport::save(std::ostream &t)
{
	int cache_size = cache.size();

	t.write(reinterpret_cast<char *>(&cache_size), sizeof(int));

	for (auto addr : cache)
	{
		auto key = addr.first;
		auto value = addr.second;

		t.write(reinterpret_cast<char *>(&key), sizeof(key));
		t.write(reinterpret_cast<char *>(&value), sizeof(value));
	}

	return MPI_SUCCESS;
}

int UDPTransport::load(std::istream &t)
{
	struct sockaddr_in addr;
	int cache_size;
	long int rank;

	t.read(reinterpret_cast<char *>(&cache_size), sizeof(int));

	while (cache_size)
	{
		t.read(reinterpret_cast<char *>(&rank), sizeof(rank));
		t.read(reinterpret_cast<char *>(&addr), sizeof(addr));
		cache.insert({rank, addr});
		cache_size--;
	}

	return MPI_SUCCESS;
}

int UDPTransport::halt()
{
	return MPI_SUCCESS;
}

int UDPTransport::cleanup()
{
	return MPI_SUCCESS;
}

}
