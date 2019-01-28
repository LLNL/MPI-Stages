#ifndef __EXAMPI_BASIC_TCP_H
#define __EXAMPI_BASIC_TCP_H

#include <basic.h>
#include <address.h>
#include <netinet/tcp.h>
#include <climits>
#include <fcntl.h>

namespace exampi
{

class TCPSocket
{
private:
	int sd;

public:
	TCPSocket()
	{
		sd = socket(AF_INET, SOCK_STREAM, 0);
		if (sd < 0)
		{
			//std::cout << "ERROR: creating listening socket\n";
		}
		int flag = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
		int nagle = 1;
		setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char *)&nagle, sizeof(int));
	}

	void bindPort(uint16_t port)
	{
		struct sockaddr_in inaddr;
		inaddr.sin_family = AF_INET;
		inaddr.sin_addr.s_addr = INADDR_ANY;
		inaddr.sin_port = htons(port);

		if (bind(sd, (struct sockaddr *)&inaddr, sizeof(inaddr)) < 0)
		{
			//std::cout << "ERROR: bind failed\n";
		}

		if (listen(sd, SOMAXCONN) < 0)
		{
			//std::cout << "ERROR: listening for connection\n";
		}
		int flags;
		if ((flags = fcntl(sd, F_GETFL, 0)) < 0)
		{
			//std::cout << "ERROR: failed fcntl\n";
		}
		else
		{
			flags |= O_NONBLOCK;
			if (fcntl(sd, F_SETFL, flags) < 0)
			{
				//std::cout << "ERROR: failed fcntl non blocking\n";
			}
		}
	}

	int getFd()
	{
		return sd;
	}
	void destroy()
	{
		close(sd);
	}
};

class TCPMessage
{
private:
	struct msghdr hdr;
	std::vector<struct iovec> iov;
public:
	TCPMessage() : iov()
	{
		hdr.msg_control = NULL;
		hdr.msg_controllen = 0;
	}

	TCPMessage(std::vector<struct iovec> i) : iov(i)
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
		//std::cout << "length of message" << iov[1].iov_len << " header "<<
		//iov[0].iov_len << "\n";

		hdr.msg_iov = iov.data();
		hdr.msg_iovlen = iov.size();
	}
	void updateHeader(Address &addr)
	{
		updateHeader();
		hdr.msg_name = addr.get();
		hdr.msg_namelen = addr.size();
	}


	void send(int &sock, Address &addr)
	{
		updateHeader(addr);

		// debug output
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(addr.get()->sin_addr), str, INET_ADDRSTRLEN);
		//std::cout << "\tbasic::Transport::tcp::send\n"
		//        << "\t\t" << hdr.msg_iovlen << " iovecs\n"
		//       << "\t\t" << str << "\n";

		//ssize_t length = sendmsg(sock, &hdr, 0);
		sendmsg(sock, &hdr, 0);
		//std::cout << "Send to TCP " << length << "\n";
	}
	ssize_t receive(int &sock)
	{
		//std::cout << debug() << "basic::Transport::tcp::recv\n";
		updateHeader();
		ssize_t length = recvmsg(sock, &hdr, MSG_WAITALL);
		//std::cout << "Received from TCP " << length << "\n";
		//std::cout << debug() << "basic::Transport::tcp::recv exiting\n";
		return length;
	}
	ssize_t peek(int &sock)
	{
		updateHeader();
		ssize_t len = recvmsg(sock, &hdr, MSG_PEEK);
		return len;
	}
};

}//exampi
#endif
