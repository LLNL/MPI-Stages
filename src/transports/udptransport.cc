#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include "transports/udptransport.h"
#include "universe.h"
#include "config.h"

namespace exampi
{

int UDPProtocolMessage::pack(const Request_ptr request)
{
	// NOTE this is temporary packing
	void* data_begin = &payload[0];
	
	debug("packing message " << *(int*)data_begin);

	// TODO fix this
	memcpy(data_begin, request->payload.buffer, sizeof(int));

	debug("packed message");

	return MPI_SUCCESS;
}

int UDPProtocolMessage::unpack(Request_ptr request) const
{
	// NOTE this is temporary unpacking
	debug("unpacking called");

	const void* data_begin = &payload[0];

	debug("unpacking message");
	
	// TODO fix this cast
	// and the rest
	memcpy((void*)request->payload.buffer, data_begin, sizeof(int));

	debug("unpacked message " << *(int*)data_begin);
	
	return MPI_SUCCESS;
}

UDPTransport::UDPTransport() : message_pool(128)
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
		debug("ERROR: socket creation failed: " << socket_recv);

	// setsockopt to reuse address
	//int opt = 1;
	//if(setsockopt(server_recv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 

	// bind
	Universe& universe = Universe::get_root_universe();

	// garuntees no local collision
	int port = std::stoi(std::string(std::getenv("EXAMPI_UDP_TRANSPORT_BASE"))) + universe.rank;
	debug("udp transport port: " << port);
	
	struct sockaddr_in address_local;
	address_local.sin_family = AF_INET;
	address_local.sin_addr.s_addr = INADDR_ANY;
	address_local.sin_port = htons(port);

	if(bind(socket_recv, (sockaddr*)&address_local, sizeof(address_local)) < 0)
	{
		debug("ERROR: socket binding failed");
	}

	// prepare msg header
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;

	// cache remote addresses
	Config& config = Config::get_instance();

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
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

ProtocolMessage_uptr UDPTransport::allocate_protocol_message()
{
	std::lock_guard<std::recursive_mutex> lock(guard);

	UDPProtocolMessage* ptr = message_pool.allocate();

	return std::unique_ptr<ProtocolMessage, std::function<void(ProtocolMessage*)>>(ptr, [this](ProtocolMessage* ptr)
	{
		this->message_pool.deallocate(dynamic_cast<UDPProtocolMessage*>(ptr));
	}); 
}

const ProtocolMessage_uptr UDPTransport::ordered_recv()
{
	// early exit test
	char test;
	ssize_t size = recv(socket_recv, &test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
	if(size <= 0)
		return ProtocolMessage_uptr(nullptr);

	// then commit
	std::lock_guard<std::recursive_mutex> lock(guard);

	// check again, that the data has not been taken by another thread
	debug("double checking work availability");

	size = recv(socket_recv, &test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
	if(size <= 0)
	{
		debug("failed recheck for work");
		return ProtocolMessage_uptr(nullptr);
	}

	debug("allocating protocol message");
	ProtocolMessage_uptr msg = allocate_protocol_message();

	// fill iov
	debug("iov processing");
	iovec msg_iov;
	msg_iov.iov_base = msg.get();
	msg_iov.iov_len = sizeof(UDPProtocolMessage);

	hdr.msg_iov = &msg_iov;
	hdr.msg_iovlen = 1;

	// clear source
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	debug("receiving message");
	int err = recvmsg(socket_recv, &hdr, 0);
	if(err <= 0)
	{
		debug("socket recv error " << err);
		return ProtocolMessage_uptr(nullptr);
	}
	debug("successful receive " << ((UDPProtocolMessage*)msg.get())->payload[0]);
	debug("protocolmessage address: " << msg.get());

	return std::move(msg);
}

int UDPTransport::reliable_send(ProtocolMessage_uptr message)
{
	std::lock_guard<std::recursive_mutex> lock(guard);

	debug("assembling udp msg");

	// fill iov
	iovec msg_iov;
	msg_iov.iov_base = message.get();
	msg_iov.iov_len = sizeof(UDPProtocolMessage);
	
	hdr.msg_iov = &msg_iov;
	hdr.msg_iovlen = 1;

	// fill destination
	// TODO need world rank not communicator rank
	sockaddr_in &addr = cache[message->envelope.destination];
	hdr.msg_name = &addr;
	hdr.msg_namelen = sizeof(sockaddr_in);
	
	debug("sending message");
	int err = sendmsg(socket_recv, &hdr, 0);
	if(err <= 0)
		return MPI_ERR_RELIABLE_SEND_FAILED;
	
	debug("sent message: " << ((UDPProtocolMessage*)message.get())->payload[0]);

	return MPI_SUCCESS;
}


const std::map<Protocol, size_t> &UDPTransport::provided_protocols() const
{
	// TODO actually give protocols/sizes 
	return std::map<Protocol, size_t>();
}

}
