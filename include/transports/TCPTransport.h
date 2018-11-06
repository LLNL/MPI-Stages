#ifndef __EXAMPI_BASIC_TCPTRANSPORT_H
#define __EXAMPI_BASIC_TCPTRANSPORT_H

#include <basic.h>
#include <sys/time.h>
#include "basic/tcp.h"

namespace exampi
{
namespace basic
{
class TCPTransport : public exampi::i::Transport
{
private:
	std::string address;
	std::unordered_map<int,Address> endpoints;
	uint16_t port;
	tcp::Socket tcpListenSocket;
	std::unordered_map<int, int> clientSocket; // <rank,sd>
	fd_set readfds;
	int max_fd;

public:
	TCPTransport() : endpoints(), tcpListenSocket(), clientSocket() {;};

	virtual void init()
	{
		tcpListenSocket.bindPort(8080);

		for (int i = 0; i < exampi::global::worldSize; i++)
		{
			clientSocket[i] = 0;
		}
		FD_ZERO(&readfds);
		FD_SET(tcpListenSocket.getFd(), &readfds);
		max_fd = tcpListenSocket.getFd();
	}

	virtual void finalize()
	{
		tcpListenSocket.destroy();
	}

	virtual void init(std::istream &t)
	{
		init();
	}

	virtual size_t addEndpoint(const int rank, const std::vector<std::string> &opts)
	{
		uint16_t port = std::stoi(opts[1]);
		// TODO:  see basic/udp.h; need move constructor to avoid copy here
		Address addr(opts[0], port);

		std::cout << "\tAssigning " << rank << " to " << opts[0] << ":" << port << "\n";
		endpoints[rank] = addr;
		return endpoints.size();
	}

	virtual std::future<int> send(std::vector<struct iovec> iov, int dest, MPI_Comm comm)
	{
		std::cout << "\tbasic::Transport::send(..., " << dest <<", " << comm << ")\n";
		int sd;
		Address addr = endpoints[dest];
		if (clientSocket[dest] > 0)
		{
			sd = clientSocket[dest];
		}
		else
		{
			sd = socket(AF_INET, SOCK_STREAM, 0);
			if (sd < 0)
			{
				std::cout << "ERROR: creating client socket\n";
			}
			if (connect(sd, (struct sockaddr *) (addr.get()), sizeof(addr)) < 0)
			{
				std::cout << "ERROR: connecting socket\n";
			}
			clientSocket[dest] = sd;
		}

		tcp::Message msg(iov);
		msg.send(sd, endpoints[dest]);
		return std::promise<int>().get_future();
	}

	virtual std::future<int> receive(std::vector<struct iovec> iov, MPI_Comm comm, ssize_t *count)
	{
		std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
		std::cout << debug() << "\tiov says size is " << iov.size() << std::endl;
		std::cout << debug() << "\t ------ " << std::endl;
		tcp::Message msg(iov);

		std::cout << debug() << "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;
		for (auto c : clientSocket)
		{
			int sd = c.second;
			if (FD_ISSET(sd, &readfds))
			{
				*count = msg.receive(sd);
				break;
			}
		}
		std::cout << debug() << "basic::Transport::receive returning" << std::endl;
		return std::promise<int>().get_future();
	}

	virtual int cleanUp(MPI_Comm comm)
	{
		std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
		char buffer[2];
		struct sockaddr_storage src_addr;

		struct iovec iov[1];
		iov[0].iov_base=buffer;
		iov[0].iov_len=sizeof(buffer);

		struct msghdr message;
		message.msg_name=&src_addr;
		message.msg_namelen=sizeof(src_addr);
		message.msg_iov=iov;
		message.msg_iovlen=1;
		message.msg_control=0;
		message.msg_controllen=0;


		std::cout << debug() << "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;

		std::cout << debug() << "basic::Transport::udp::recv\n";

		int sd = clientSocket[exampi::global::rank];
		recvmsg(sd, &message, MSG_WAITALL);
		FD_ZERO(&readfds);
		FD_SET(tcpListenSocket.getFd(), &readfds);
		max_fd = tcpListenSocket.getFd();
		clientSocket.clear(0);
		for (int i = 0; i < exampi::global::worldSize; i++)
		{
			clientSocket[i] = 0;
		}
		std::cout << debug() << "basic::Transport::udp::recv exiting\n";
		std::cout << debug() << "basic::Transport::receive returning" << std::endl;
		return 0;
	}

	virtual int peek(std::vector<struct iovec> iov, MPI_Comm comm)
	{
		tcp::Message msg(iov);
		int sd;
		int newClient;
		for (auto sd : clientSocket)
		{
			if (sd.second > 0)
			{
				FD_SET(sd.second, &readfds);
			}
			if (sd.second > max_fd)
			{
				max_fd = sd.second;
			}
		}
		int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
		if (activity < 0 && errno != EINTR)
		{
			std::cout << "ERROR: In select\n";
			return 1;
		}
		if (FD_ISSET(tcpListenSocket.getFd(), &readfds))
		{
			//accept
			sockaddr_in client;
			socklen_t clientlen;
			clientlen = sizeof(client);
			newClient = accept(tcpListenSocket.getFd(), (struct sockaddr *) &client, &clientlen);
			if (newClient < 0)
			{
				std::cout << "ERROR: In accept new connection\n";
			}
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN);
			std::cout << "client ip "<< str << "\n";

			for (auto ip : endpoints)
			{
				char s[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(ip.second.get()->sin_addr), s, INET_ADDRSTRLEN);
				if (std::string(str).compare(std::string(s)) == 0)
				{
					clientSocket[ip.first] = newClient;
				}
			}
			return 1;
		}
		// I/O in some other socket
		else
		{
			for (auto c : clientSocket)
			{
				sd = c.second;
				if (FD_ISSET(sd, &readfds))
				{
					int len = msg.peek(sd);
					//check if close request
					if (len == 0)
					{
						close(sd);
						for (auto c1 : clientSocket)
						{
							int descriptor = c1.second;
							if (descriptor == sd) continue;
							if (descriptor > 0)
							{
								close(descriptor);
							}
						}
						FD_ZERO(&readfds);
						FD_SET(tcpListenSocket.getFd(), &readfds);
						max_fd = tcpListenSocket.getFd();
						clientSocket.clear(0);
						for (int i = 0; i < exampi::global::worldSize; i++)
						{
							clientSocket[i] = 0;
						}
						return 1;
					}
				}
			}
		}

		return 0;
	}

	virtual int save(std::ostream &t)
	{
		// save endpoints
		int epsz = endpoints.size();
		//std::cout << "size: " << epsz << "\n";
		t.write(reinterpret_cast<char *>(&epsz), sizeof(int));
		for(auto i : endpoints)
		{
			auto key = i.first;
			auto val = i.second;
			t.write(reinterpret_cast<char *>(&key), sizeof(key));
			t.write(reinterpret_cast<char *>(&val), sizeof(val));
		}
		return 0;
	}

	virtual int load(std::istream &t)
	{
		init();
		// load endpoints
		int epsz;
		int rank;
		Address addr;
		t.read(reinterpret_cast<char *>(&epsz), sizeof(int));
		//std::cout << "size: " << epsz << "\n";
		while(epsz)
		{
			t.read(reinterpret_cast<char *>(&rank), sizeof(rank));
			t.read(reinterpret_cast<char *>(&addr), sizeof(addr));
			endpoints[rank] = addr;
			epsz--;
		}
		return 0;
	}
};
}
}

#endif
