#include "transports/udp.h"

namespace exampi
{

Socket::Socket()
{
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

Socket::~Socket()
{
	close(fd);
}

void Socket::bindPort(uint16_t port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		// TODO needs errorcode!
		std::cout << "WARNING:  Bind failed\n";
		exit(234);
	}
}

int Socket::getFd()
{
	return fd;
}

void Socket::destroy()
{
	close(fd);
}

//Message::Message() : iov()
//{
//	hdr.msg_control = NULL;
//	hdr.msg_controllen = 0;
//}

Message::Message(std::vector<struct iovec> &i) : iov(i)
{
	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;
}

void Message::addBuf(exampi::Buf *b)
{
	iov.push_back(b->iov());
}

void Message::updateHeader()
{
	debug("updateHeader length of message" << iov[1].iov_len << " header "<<
	        iov[0].iov_len );
	hdr.msg_iov = iov.data();
	hdr.msg_iovlen = iov.size();
}

void Message::updateHeader(Address &addr)
{
	updateHeader();
	hdr.msg_name = addr.get();
	hdr.msg_namelen = addr.size();
}

void Message::send(Socket &sock, Address &addr)
{
	updateHeader(addr);

	// debug output
	char str[INET_ADDRSTRLEN];

	// XXX
	inet_ntop(AF_INET, &(addr.get()->sin_addr), str, INET_ADDRSTRLEN);

	debug("basic::Transport::udp::send\t" << hdr.msg_iovlen << " iovecs\t" <<
	        str);

	// TODO make use of length to check
	//ssize_t length = sendmsg(sock.getFd(), &hdr, 0);
	sendmsg(sock.getFd(), &hdr, 0);
	//debug("Send to UDP " << length);
}

ssize_t Message::receive(Socket &sock)
{
	debug("basic::Transport::udp::recv");
	updateHeader();
	ssize_t length = recvmsg(sock.getFd(), &hdr, MSG_WAITALL);
	debug("Received from UDP " << length);
	debug("basic::Transport::udp::recv exiting");
	return length;
}

void Message::peek(Socket &sock)
{
	debug("msg::peek");

	updateHeader();

	debug("msg::peek recvmsg");

	recvmsg(sock.getFd(), &hdr, MSG_WAITALL | MSG_PEEK);

	debug("msg::peek recvmsg completed");
}

}
