// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

#include <sstream>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "daemon.h"
#include "debug.h"
#include "universe.h"

namespace exampi
{

Daemon &Daemon::get_instance()
{
	// Create singleton object if first time
	static Daemon instance;

	return instance;
}

Daemon::Daemon()
{
	Universe &universe = Universe::get_root_universe();

	// initiate socket
	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(this->sock < 0)
	{
		debug("rank " << universe.rank << " daemon socket failed.");
		return;
	}

	// find hostname
	char hostname[1024];
	gethostname(hostname, 1024);
	debug("getting hostname as " << hostname);

	struct in_addr *host = (struct in_addr *)gethostbyname(
	                           hostname)->h_addr_list[0];
	debug("getting local ip as " << inet_ntoa(*host));

	// set daemon sock addr
	debug("daemon port " << std::string(std::getenv("EXAMPI_HEAD_DAEMON_PORT")));
	int daemon_port = std::stoi(std::string(
	                                std::getenv("EXAMPI_HEAD_DAEMON_PORT")));

	this->daemon.sin_family = AF_INET;
	this->daemon.sin_port = htons(daemon_port);
	this->daemon.sin_addr = *host;
	debug("generated daemon sockaddr_in");

	bool unconnected = true;
	do
	{
		debug("attempting connection to daemon");
		if(connect(this->sock, (const sockaddr *)&this->daemon,
		           sizeof(this->daemon)) == -1)
		{
			// failed to connect to head daemon
			debug("failed to establish tcp connection, waiting 250ms");

			// sleep for 250 ms
			usleep(250 * 1000);
		}
		else
		{
			unconnected = false;
		}
	}
	while(unconnected);
	debug("tcp connection to daemon established");
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

	debug("daemon: barrier complete");
	return err;
}

int Daemon::abort()
{
	Universe &universe = Universe::get_root_universe();

	std::stringstream packet;
	packet << "abort ";
	packet << universe.rank << " ";
	packet << universe.epoch;

	// add padding
	while(packet.str().length() < 64)
	{
		packet << ' ';
	}

	debug("send_abort:" << packet.str() << " " << packet.str().length());

	int err = send(packet.str());

	return MPI_SUCCESS;
}

int Daemon::send_barrier_ready()
{
	Universe &universe = Universe::get_root_universe();

	std::stringstream packet;
	packet << "barrier ";
	packet << universe.rank << " ";
	packet << getpid();

	// add padding
	while(packet.str().length() < 64)
	{
		packet << ' ';
	}

	debug("send_barrier_ready:" << packet.str() << " " << packet.str().length());

	return send(packet.str());
}

int Daemon::recv_barrier_release()
{
	Universe &universe = Universe::get_root_universe();

	debug("in recv_barrier_release " << universe.rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, 0);
	debug("rank recv barrier release " << err << " msg " << msg);
	if(err != 64)
	{
		debug("rank " << universe.rank << " msg failed, retrying barrier");
		return 1;
	}

	debug("rank " << universe.rank << " recv barrier " << std::string(msg));
	debug("rank " << universe.rank << " " << std::string(msg));

	if(std::string(msg).compare(std::string("release")) == 0)
		return 0;
	else
	{
		debug("release invalid compare " << std::string(msg));
	}

	return 1;
}

int Daemon::send_clean_up()
{
	Universe &universe = Universe::get_root_universe();

	std::stringstream packet;
	packet << "cleanup ";
	packet << universe.rank << " ";
	packet << universe.epoch;

	// add padding
	while(packet.str().length() < 64)
	{
		packet << ' ';
	}

	debug("send_clean_up:" << packet.str() << " " << packet.str().length());

	return send(packet.str());
}

int Daemon::wait_commit()
{
	Universe &universe = Universe::get_root_universe();

	debug("in wait_commit " << universe.rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, 0);
	debug("rank recv commit " << err << " msg " << msg);
	if(err != 64)
	{
		debug("rank " << universe.rank << " msg failed");
		return 1;
	}

	debug("rank " << universe.rank << " recv commit " << std::string(msg));

	// assign epoch number
	std::string msgstr(msg);
	universe.epoch = std::stoi(msgstr.substr(7));

	// note might need to load mpi checkpoint data
	// note this is an assumption that we can revert previous checkpoint
	// note if corruption exists this will not work

	return MPI_SUCCESS;
}

int Daemon::send(std::string packet)
{
	if(this->sock < 0)
	{
		debug("ERROR this->sock < 0");
		return -35434;
	}

	// check if socket is open
	else if(packet.length() > 64)
	{
		debug("ERROR packet.legnth > 64");
		return -54563;
	}

	// send packet to daemon
	debug("send packet to daemon: " << packet.c_str());
	return ::send(this->sock, packet.c_str(), packet.length(), 0);
}

std::string Daemon::recv()
{
	return std::string("");
}

} // ::exampi
