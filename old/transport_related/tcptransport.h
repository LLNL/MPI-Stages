#ifndef __EXAMPI_BASIC_TCPTRANSPORT_H
#define __EXAMPI_BASIC_TCPTRANSPORT_H

//#include <basic.h>
//#include <sys/time.h>
//#include "transports/tcp.h"

namespace exampi
{

//class TCPTransport: public exampi::Transport
//{
//private:
//	std::string address;
//	std::unordered_map<int,Address> endpoints;
//
//	TCPSocket tcpListenSocket;
//
//	std::unordered_map<int, int> clientSocket; // <rank,sd>
//	fd_set readfds;
//	int max_fd;
//
//public:
//	virtual void init();
//	virtual void finalize();
//	virtual void init(std::istream &t);
//
//	virtual std::future<int> send(std::vector<struct iovec> &iov, int dest,
//	                              MPI_Comm comm);
//	virtual std::future<int> receive(std::vector<struct iovec> &iov, MPI_Comm comm,
//	                                 ssize_t *count);
//	virtual int cleanUp(MPI_Comm comm);
//
//	virtual int peek(std::vector<struct iovec> &iov, MPI_Comm comm);
//
//	virtual int save(std::ostream &t);
//	virtual int load(std::istream &t);
//};

}

#endif
