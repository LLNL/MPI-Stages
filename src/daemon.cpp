#include "daemon.h"
#include "global.h"
#include "ExaMPI.h"

#include <sstream>
#include <iostream>
#include <cstdlib>

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
	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(this->sock < 0)
	{
		debugpp("rank " << exampi::rank << " daemon socket failed.");
		return;
	}

	//int reuse = 1;
	//setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	//int buf_size = 0;
	//setsockopt(this->sock, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(int));
	//setsockopt(this->sock, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(int));

	// find hostname
	char hostname[1024];
	gethostname(hostname, 1024);
	debugpp("getting hostname as " << hostname);

	struct in_addr* host = (struct in_addr*)gethostbyname(hostname)->h_addr_list[0];
	debugpp("getting local ip as " << inet_ntoa(*host));

	
	// bind to local
	//debugpp("recv mpi port " << std::string(std::getenv("EXAMPI_MPI_PORT")));
	//int mpi_port = std::stoi(std::string(std::getenv("EXAMPI_MPI_PORT")));
	//this->local.sin_family = AF_INET;
	//this->local.sin_port = htons(mpi_port);
	////this->local.sin_addr.s_addr = inet_addr(h_addr_list[0]);
	//this->local.sin_addr = *host;
	//// TODO check error code
	////int err = bind(this->sock, (sockaddr *)&this->local, sizeof(this->local));
	//int err = bind(this->sock, (sockaddr *)&this->local, sizeof(this->local));
	//if(err != 0)
	//{
	//	debugpp("failed to bind port");
	//	exit(124);
	//}

	// set daemon sock addr
	debugpp("daemon port " << std::string(std::getenv("EXAMPI_HEAD_DAEMON_PORT")));
	int daemon_port = std::stoi(std::string(std::getenv("EXAMPI_HEAD_DAEMON_PORT")));

	this->daemon.sin_family = AF_INET;
	this->daemon.sin_port = htons(daemon_port);
	//this->daemon.sin_addr.s_addr = inet_addr(ip->h_addr_list[0]);
	this->daemon.sin_addr = *host;
	debugpp("generated daemon sockaddr_in");

	bool unconnected = true;
	do
	{
		debugpp("attempting connection to daemon");
		if(connect(this->sock, (const sockaddr*)&this->daemon, sizeof(this->daemon)) == -1)
		{
			// failed to connect to head daemon
			debugpp("failed to establish tcp connection, waiting 250ms");
			
			// sleep for 250 ms
			usleep(250 * 1000);
		}
		else
		{
			unconnected = false;
		}
	}
	while(unconnected);
	debugpp("tcp connection to daemon established");
}

Daemon::~Daemon()
{
	// destroy socket
	::close(this->sock);
}

int Daemon::barrier() 
{
	int err;

	// send barrier
	err = send_barrier_ready();

	err = recv_barrier_release();

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
		return 1;
	}

	debugpp("rank " << exampi::rank << " recv barrier " << std::string(msg));
	debugpp("rank " << exampi::rank << " " << std::string(msg));

	if(std::string(msg).compare(std::string("release")) == 0)
		return 0;
	else
	{
		debugpp("release invalid compare " << std::string(msg));
	}

	return 1;
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
	{
		debugpp("ERROR this->sock < 0");
		return -35434;
	}

	// check if socket is open
	else if(packet.length() > 64)
	{ 
		debugpp("ERROR packet.legnth > 64");
		return -54563;
	}

	// send packet to daemon
	debugpp("send packet to daemon: " << packet.c_str());
	return ::send(this->sock, packet.c_str(), packet.length(), 0);
}

std::string Daemon::recv()
{
	return std::string("");
}

} // ::exampi
