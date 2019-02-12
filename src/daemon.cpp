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

Daemon &Daemon::get_instance()
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
		debug("rank " << exampi::rank << " daemon socket failed.");
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

	debug("send_barrier_ready:" << packet.str() << " " << packet.str().length());

	return send(packet.str());
}

int Daemon::recv_barrier_release()
{
	debug("in recv_barrier_release " << exampi::rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, 0);
	debug("rank recv barrier release " << err << " msg " << msg);
	if(err != 64)
	{
		debug("rank " << exampi::rank << " msg failed, retrying barrier");
		return 1;
	}

	debug("rank " << exampi::rank << " recv barrier " << std::string(msg));
	debug("rank " << exampi::rank << " " << std::string(msg));

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
	std::stringstream packet;
	packet << "cleanup ";
	packet << exampi::rank << " ";
	packet << getpid() << " ";
	packet << exampi::epoch;

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
	debug("in wait_commit " << exampi::rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, 0);
	debug("rank recv commit " << err << " msg " << msg);
	if(err != 64)
	{
		debug("rank " << exampi::rank << " msg failed");
		return 1;
	}

	debug("rank " << exampi::rank << " recv commit " << std::string(msg));

	// assign epoch number
	std::string msgstr(msg);
	exampi::epoch = std::stoi(msgstr.substr(7));

	// TODO might need to load mpi checkpoint data

	return 0;
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
