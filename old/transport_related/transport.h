#ifndef __EXAMPI_BASIC_TRANSPORT_H
#define __EXAMPI_BASIC_TRANSPORT_H

namespace exampi
{

//class BasicTransport : public Transport
//{
//private:
//	std::string address;
//	std::unordered_map<int, Address> endpoints;
//	//uint16_t port;
//
//	Socket recvSocket;
//
//	//int tcpSock;
//
//	int base_port;
//	int port;
//
//public:
//	BasicTransport();
//
//	void init();
//
//	void init(std::istream &t);
//
//	void finalize();
//
//	std::future<int> send(std::vector<struct iovec> &iov, int dest, MPI_Comm comm);
//
//	std::future<int> receive(std::vector<struct iovec> &iov, MPI_Comm comm,
//	                         ssize_t *count);
//
//	int cleanUp(MPI_Comm comm);
//
//	int peek(std::vector<struct iovec> &iov, MPI_Comm comm);
//
//	int save(std::ostream &t);
//
//	int load(std::istream &t);
//};

} // exampi
#endif //guard