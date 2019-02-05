#include "transports/udptransport.h"

namespace exampi
{

UDPTransport::UDPTransport() : Transport()
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
	{
		// TODO handle error
	}

	// setsockopt to reuse address
	int opt = 1;
	if (setsockopt(server_recv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 

	// bind
	Universe& universe = Universe::get_root_universe();

	// garuntees no local collision
	int port = std::stoi(std::string(std::getenv("EXAMPI_UDP_TRANSPORT_BASE"))) + universe.rank;
	
	struct sockaddr_in address_local;
	address_local.sin_family = AF_INET;
	address_local.sin_addr.s_addr = INADDR_ANY;
	address_local.sin_port = htons(port);

	if(bind(server_recv, address_local, sizeof(address_local)) < 0)
	{
		// TODO handle error
	}

	// prepare msg header
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = NULL;
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

bool peek(ProtocolMessage_uptr &message)
{
	// early exit test
	char test;
	ssize_t size = recv(socket_recv, &test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
	if(size <= 0)
		return false;

	std::lock_guard lock(guard);

	// check again, that the data has not been taken by another thread
	ssize_t size = recv(socket_recv, &test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
	if(size <= 0)
		return false;

	ProtocolMessage_uptr msg = allocate_protocol_message();

	// fill iov
	iovec msg_iov;
	msg_iov.iov_base = msg.get();
	msg_iov.iov_len = sizeof(ProtocolMessage);

	hdr.msg_iov = &msg_iov;
	hdr.msg_iovlen = 1;

	// clear source
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;

	int err = recvmsg(socket_recv, &hdr, NULL);
	// TODO handle error

	message = std::move(msg);

	return true;
}

//ProtocolMessage_uptr UDPTransport::fetch(ProtocolEnvelope &envelope)
//{
//	// lock is already owned, will unlock at the end of function
//	std::lock_guard<std::mutex> lock(guard, std::adopt_lock);
//
//	// create ProtocolMessage instance
//	ProtocolMessage_uptr ptr = allocate_protocol_message();
//
//	// fill iov
//	iovec protocol_message_iov;
//	protocol_message_iov.iov_base = ptr.get();
//	protocol_message_iov.iov_len = sizeof(ProtocolMessage);
//	
//	hdr.msg_iov = &protocol_message_iov;
//	hdr.msg_iovlen = 1;
//	
//	// recv message via udp
//	ssize_t err = recvmsg(socket_recv, &hdr, MSG_WAITALL);
//
//	if(err <= 0)
//	{
//		// TODO handle error
//	}
//
//	return ptr;
//}
//
//bool UDPTransport::fill(ProtocolEnvelope &envelope, Payload &payload)
//{
//	// lock is already owned
//	std::lock_guard<std::mutex> lock(guard, std::adopt_lock);
//
//	// TODO copy what ever the message is into the 
//	
//}

int UDPTransport::reliable_send(ProtocolMessage_uptr message)
{
	std::lock_guard lock(guard);

	// fill iov
	iovec msg_iov;
	msg_iov.iov_base = message.get();
	msg_iov.iov_len = sizeof(ProtocolMessage);
	
	hdr.msg_iov = &msg_iov;
	hdr.msg_iovlen = 1;

	// fill destination
	hdr.msg_name = "";
	hdr.msg_namelen = 0;

	int err = sendmsg(socket_recv, &hdr, NULL);
	// TODO handle error
	
	return MPI_SUCCESS;
}

}
