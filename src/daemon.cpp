#include "daemon.h"
#include "global.h"
#include "ExaMPI.h"

#include <sstream>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace exampi
{

// TODO these ports should be given in the environment variables
// handle through config
//int DAEMON_UDP_PORT = 50000;
//int PROC_BASE_UDP_PORT = 40000;

Daemon& Daemon::get_instance()
{
	// Create singleton object if first time
	static Daemon instance;

	return instance;
}

Daemon::Daemon()
{
	// initiate socket
	this->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(this->sock < 0)
	{
		debugpp("rank " << exampi::rank << " daemon socket failed.");
		return;
	}

	// find hostname
	char hostname[1024];
	gethostname(hostname, 1024);

	struct in_addr* host = (struct in_addr*)gethostbyname(hostname)->h_addr_list[0];
	
	// bind to local
	debugpp("recv mpi port " << std::string(std::getenv("EXAMPI_MPI_PORT")));
	int mpi_port = std::stoi(std::string(std::getenv("EXAMPI_MPI_PORT")));
	this->local.sin_family = AF_INET;
	this->local.sin_port = htons(mpi_port);
	//this->local.sin_addr.s_addr = inet_addr(h_addr_list[0]);
	this->local.sin_addr = *host;
	// TODO check error code
	//int err = bind(this->sock, (sockaddr *)&this->local, sizeof(this->local));
	bind(this->sock, (sockaddr *)&this->local, sizeof(this->local));

	// set recv timeout
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100 * 1000; // 100ms
	setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	// set daemon sock addr
	debugpp("daemon port " << std::string(std::getenv("EXAMPI_DAEMON_PORT")));
	int daemon_port = std::stoi(std::string(std::getenv("EXAMPI_DAEMON_PORT")));
	this->daemon.sin_family = AF_INET;
	this->daemon.sin_port = htons(daemon_port);
	//this->daemon.sin_addr.s_addr = inet_addr(ip->h_addr_list[0]);
	this->daemon.sin_addr = *host;
}

Daemon::~Daemon()
{
	// destroy socket
	close(this->sock);
}

int Daemon::barrier() 
{
	int err;

	// send barrier
	err = send_barrier_ready();

	while(recv_barrier_release())
	{
		// resend barrier
		err = send_barrier_ready();
	}


	debugpp("daemon: barrier complete");
	return err;	
}

int Daemon::send_barrier_ready()
{
	std::stringstream packet;
	packet << "barrier ";
	packet << exampi::rank << " ";
	packet << getpid();

	// add padding
	while(packet.str().length() < 64)
	{
		packet << ' ';
	}

	debugpp("send_barrier_ready:" << packet.str() << " " << packet.str().length());
	
	return send(packet.str());
}

int Daemon::recv_barrier_release()
{
	debugpp("in recv_barrier_release " << exampi::rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, NULL);
	debugpp("rank recv barrier release " << err << " msg " << msg);
	if(err != 64)
	{
		debugpp("rank " << exampi::rank << " msg failed, retrying barrier");
		return err;
	}

	debugpp("rank " << exampi::rank << " recv barrier " << std::string(msg));

	if(std::string(msg) == std::string("release"))
		return 0;

	return err;
}

int Daemon::send_clean_up()
{
	std::stringstream packet;
	packet << exampi::rank << " ";
	packet << getpid() << " ";
	packet << exampi::epoch;

	// add padding
	while(packet.str().length() < 64)
	{
		packet << ' ';
	}

	debugpp("send_clean_up:" << packet.str() << " " << packet.str().length());

	return send(packet.str());
}

int Daemon::send(std::string packet)
{
	if(this->sock < 0)
		return -35434;

	// check if socket is open
	else if(packet.length() > 64)
		return -54563;

	// send packet to daemon
	return sendto(this->sock, packet.c_str(), packet.length(), 0,
	              (sockaddr *)&this->daemon, sizeof(this->daemon));
}

std::string Daemon::recv()
{
	return std::string("");
}

} // ::exampi
