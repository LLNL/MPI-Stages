#ifndef __EXAMPI_BASIC_TRANSPORT_H
#define __EXAMPI_BASIC_TRANSPORT_H

#include <basic.h>
#include "basic/udp.h"
//#include "basic/tcp.h"

namespace exampi {
namespace basic {

class Transport : public exampi::i::Transport
{
private:
	std::string address;
	std::unordered_map<int,Address> endpoints;
	uint16_t port;
	udp::Socket recvSocket;

public:
	Transport() : endpoints(), recvSocket() {;};

	virtual void init()
	{
		recvSocket.bindPort(8080);
	}

	virtual void init(std::istream &t)
	{
		init();
	}

	virtual void finalize() {
		recvSocket.destroy();
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
		udp::Socket s;
		udp::Message msg(iov);

		msg.send(s, endpoints[dest]);
		return std::promise<int>().get_future();
    		}

	virtual std::future<int> receive(std::vector<struct iovec> iov, MPI_Comm comm, ssize_t *count)
    		{
		std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
		std::cout << debug() << "\tiov says size is " << iov.size() << std::endl;
		std::cout << debug() << "\t ------ " << std::endl;
		udp::Message msg(iov);

		std::cout << debug() << "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;

		*count = msg.receive(recvSocket);
		std::cout << debug() << "basic::Transport::receive returning" << std::endl;
		return std::promise<int>().get_future();
    		}

	virtual int cleanUp(MPI_Comm comm) {
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

		recvmsg(recvSocket.getFd(), &message, MSG_WAITALL);
		std::cout << debug() << "basic::Transport::udp::recv exiting\n";
		std::cout << debug() << "basic::Transport::receive returning" << std::endl;
		return 0;
	}

	virtual int peek(std::vector<struct iovec> iov, MPI_Comm comm)
	{
		udp::Message msg(iov);
		msg.peek(recvSocket);
		return 0;
	}

	/*std::vector<std::string> split(std::vector<std::string> vec, std::string line) {
    	vec.clear();
    	std::size_t delim = line.find_first_of("|");
    	std::string key = line.substr(0, delim);
    	vec.push_back(key);
    	std::string val = line.substr(delim+1);
    	vec.push_back(val);
    	return vec;
    }*/

	virtual int save(std::ostream& t)
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

	virtual int load(std::istream& t)
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

} // basic
} // exampi
#endif //guard
