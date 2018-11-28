#include "transports/transport.h"
#include "transports/udp.h"

namespace exampi
{

void BasicTransport::init()
{
	debugpp("binding udp port " << this->port + exampi::rank);
	recvSocket.bindPort(this->port + exampi::rank);
}

void BasicTransport::init(std::istream &t)
{
	init();
}

void BasicTransport::finalize()
{
	recvSocket.destroy();
}

size_t BasicTransport::addEndpoint(const int rank,
                                   const std::vector<std::string> &opts)
{
	uint16_t port = std::stoi(opts[1]);
	// TODO:  see basic/udp.h; need move constructor to avoid copy here
	Address addr(opts[0], port);

	debugpp("Transport add endpoint rank " << exampi::rank << " assigning " << rank << " to " << opts[0] << ":" << port);

	endpoints[rank] = addr;
	return endpoints.size();
}

std::future<int> BasicTransport::send(std::vector<struct iovec> iov, int dest,
                                      MPI_Comm comm)
{
	debugpp("basic::Transport::send(..., " << dest << ", " << comm);

	Socket s;
	Message msg(iov);

	msg.send(s, endpoints[dest]);
	return std::promise<int>().get_future();
}

std::future<int> BasicTransport::receive(std::vector<struct iovec> iov,
        MPI_Comm comm,
        ssize_t *count)
{
	debugpp("basic::Transport::receive(...)");
	debugpp("\tiov says size is " << iov.size());
	Message msg(iov);

	debugpp("basic::Transport::receive, constructed msg, calling msg.receive");
	//msg.receive(recvSocket, tcpSock); /*For TCP transport*/
	*count = msg.receive(recvSocket);
	debugpp("basic::Transport::receive returning");
	return std::promise<int>().get_future();
}

int BasicTransport::cleanUp(MPI_Comm comm)
{
	//std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
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


	//std::cout << debug() <<
	//          "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;

	//std::cout << debug() << "basic::Transport::udp::recv\n";

	recvmsg(recvSocket.getFd(), &message, MSG_WAITALL);
	//std::cout << debug() << "basic::Transport::udp::recv exiting\n";
	//std::cout << debug() << "basic::Transport::receive returning" << std::endl;
	return 0;
}

int BasicTransport::peek(std::vector<struct iovec> iov, MPI_Comm comm)
{
	Message msg(iov);
	//msg.peek(recvSocket, tcpSock); /*For TCP transport*/
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

int BasicTransport::save(std::ostream &t)
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

int BasicTransport::load(std::istream &t)
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


}
