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
	debug("packing message " << payload[0])

	// todo avoid this copy by pointing to request buffer, datatype dependent, count dependent
	//      assemble via iov
	//
	//      options: copying, pointing
	memcpy(&payload[0], request->payload.buffer, sizeof(payload));

	debug("packed message");

	return MPI_SUCCESS;
}

int UDPProtocolMessage::unpack(Request_ptr request) const
{
	debug("unpacking called");

	memcpy((void *)request->payload.buffer, &payload[0], sizeof(payload));

	debug("request buffer " << ((int *)request->payload.buffer)[0]);

	return MPI_SUCCESS;
}

UDPTransport::UDPTransport() : message_pool(128)
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
	{
		debug("ERROR: socket creation failed: " << socket_recv);

		throw UDPTransportCreationException();
	}

	// setsockopt to reuse address
	//int opt = 1;
	//if(setsockopt(server_recv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))

	// bind
	Universe &universe = Universe::get_root_universe();

	// garuntees no local collision
	int port = std::stoi(std::string(std::getenv("EXAMPI_UDP_TRANSPORT_BASE"))) +
	           universe.rank;
	debug("udp transport port: " << port);

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
	available_protocols[Protocol::EAGER] = sizeof(UDPProtocolMessage::payload);
	available_protocols[Protocol::EAGER_ACK] = sizeof(UDPProtocolMessage::payload);
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

ProtocolMessage_uptr UDPTransport::allocate_protocol_message()
{
	std::lock_guard<std::recursive_mutex> lock(guard);

	ProtocolMessage *ptr = dynamic_cast<ProtocolMessage *>(message_pool.allocate());

	return std::unique_ptr<ProtocolMessage, std::function<void(ProtocolMessage *)>>
	       (ptr,
	        [this](ProtocolMessage *ptr)
	{
		debug("UDPProtocolMessage deallocation called");
		this->message_pool.deallocate(dynamic_cast<UDPProtocolMessage *>(ptr));
	} );
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

	UDPProtocolMessage *umsg = dynamic_cast<UDPProtocolMessage *>(msg.get());

	// fill iov
	// TODO this is where protocolmessage size is important
	debug("iov processing");
	iovec iovs[3];
	
	iovs[0].iov_base = &umsg->stage;
	iovs[0].iov_len = sizeof(umsg->stage);

	iovs[1].iov_base = &umsg->envelope;
	iovs[1].iov_len = sizeof(umsg->envelope);

	iovs[2].iov_base = &umsg->payload;
	iovs[2].iov_len = sizeof(umsg->payload);

	debug("iov data " << &msg->stage << " size " << msg->size());

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 3;

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
	debug("recv message envelope: { e " << msg->envelope.epoch << " c " << msg->envelope.context << " s " << msg->envelope.source << " d " << msg->envelope.destination << " t " << msg->envelope.tag << "}");
	debug("successful receive " << ((UDPProtocolMessage *)msg.get())->payload[0]);

	return std::move(msg);
}

int UDPTransport::reliable_send(ProtocolMessage_uptr message)
{
	std::lock_guard<std::recursive_mutex> lock(guard);

	debug("assembling udp msg");

	UDPProtocolMessage *msg = dynamic_cast<UDPProtocolMessage *>(message.get());

	// fill iov
	iovec iovs[3];
	
	iovs[0].iov_base = &msg->stage;
	iovs[0].iov_len = sizeof(msg->stage);

	iovs[1].iov_base = &msg->envelope;
	iovs[1].iov_len = sizeof(msg->envelope);

	iovs[2].iov_base = &msg->payload;
	iovs[2].iov_len = sizeof(msg->payload);

	//msg_iov.iov_base = &message->stage;
	//msg_iov.iov_len = message->size();
	//debug("iov data " << &message->stage << " size " << message->size());

	hdr.msg_iov = iovs;
	hdr.msg_iovlen = 3;

	// fill destination
	// TODO this needs to be a facility given by communicator
	// TODO translate (context, rank) -> (world_context, world_rank) -> addr
	sockaddr_in &addr = cache[message->envelope.destination];
	hdr.msg_name = &addr;
	hdr.msg_namelen = sizeof(sockaddr_in);

	debug("sending message");
	int err = sendmsg(socket_recv, &hdr, 0);
	if(err <= 0)
		return MPI_ERR_RELIABLE_SEND_FAILED;

	debug("sent message envelope: { e " << message->envelope.epoch << " c " << message->envelope.context << " s " << message->envelope.source << " d " << message->envelope.destination << " t " << message->envelope.tag << "}");
	debug("sent message: " << ((UDPProtocolMessage *)message.get())->payload[0]);

	return MPI_SUCCESS;
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
	// TODO no idea what this does, from original udp transport
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
