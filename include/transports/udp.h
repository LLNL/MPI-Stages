#ifndef __EXAMPI_BASIC_UDP_H
#define __EXAMPI_BASIC_UDP_H

#include <basic.h>
#include <address.h>

namespace exampi
{

class Socket
{
private:
	int fd;
public:
	Socket()
	{
		fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	~Socket()
	{
		close(fd);
	}
	void bindPort(uint16_t port)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			// TODO needs errorcode!
			std::cout << "WARNING:  Bind failed\n";
	}
	int getFd()
	{
		return fd;
	}
	void destroy()
	{
		close(fd);
	}
};

// WARNING:  This class creates a THIRD copy of the std::vector
// Need to write a custom container and do move semantics
class Message
{
private:
	struct msghdr hdr;
	std::vector<struct iovec> iov;

public:
	Message() : iov()
	{
		hdr.msg_control = NULL;
		hdr.msg_controllen = 0;
	}

	Message(std::vector<struct iovec> i) : iov(i)
	{
		hdr.msg_control = NULL;
		hdr.msg_controllen = 0;
		hdr.msg_name = NULL;
		hdr.msg_namelen = 0;
	}

	void addBuf(exampi::Buf *b)
	{
		iov.push_back(b->iov());
	}
	void updateHeader()
	{
		debugpp("updateHeader length of message" << iov[1].iov_len << " header "<< iov[0].iov_len );
		hdr.msg_iov = iov.data();
		hdr.msg_iovlen = iov.size();
	}
	void updateHeader(Address &addr)
	{
		updateHeader();
		hdr.msg_name = addr.get();
		hdr.msg_namelen = addr.size();
	}


	void send(Socket &sock, Address &addr)
	{
		updateHeader(addr);

		// debug output
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(addr.get()->sin_addr), str, INET_ADDRSTRLEN);
		debugpp("basic::Transport::udp::send\t" << hdr.msg_iovlen << " iovecs\t" << str);

		// TODO make use of length to check
		//ssize_t length = sendmsg(sock.getFd(), &hdr, 0);
		sendmsg(sock.getFd(), &hdr, 0);
		//debugpp("Send to UDP " << length);
	}
	ssize_t receive(Socket &sock)
	{
		debugpp("basic::Transport::udp::recv");
		updateHeader();
		ssize_t length = recvmsg(sock.getFd(), &hdr, MSG_WAITALL);
		debugpp("Received from UDP " << length);
		debugpp("basic::Transport::udp::recv exiting");
		return length;
	}
	void peek(Socket &sock)
	{
		debugpp("msg::peek");

		updateHeader();

		debugpp("msg::peek recvmsg");

		recvmsg(sock.getFd(), &hdr, MSG_WAITALL | MSG_PEEK);

		debugpp("msg::peek recvmsg completed");
	}
};

} // exampi

#endif // guard
