#include "daemon.h"
#include "global.h"
#include "ExaMPI.h"

#include <sstream>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>

namespace exampi
{

Daemon *Daemon::instance = nullptr;

std::string UDP_IP("127.0.0.1");

// TODO these ports should be given in the environment variables
// handle through config
int DAEMON_UDP_PORT = 50000;
int PROC_BASE_UDP_PORT = 40000;

Daemon::Daemon()
{
	// initiate socket
	this->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(this->sock < 0)
	{
		debugpp("rank " << exampi::rank << " daemon socket failed.");
		return;
	}

	// bind to local
	int port = PROC_BASE_UDP_PORT + exampi::rank;
	this->local.sin_family = AF_INET;
	this->local.sin_port = htons(port);
	this->local.sin_addr.s_addr = inet_addr(UDP_IP.c_str());
	int err = bind(this->sock, (sockaddr *)&this->local, sizeof(this->local));

	// set daemon sock addr
	this->daemon.sin_family = AF_INET;
	this->daemon.sin_port = htons(DAEMON_UDP_PORT);
	this->daemon.sin_addr.s_addr = inet_addr(UDP_IP.c_str());
}

Daemon::~Daemon()
{
	// destroy socket
	close(this->sock);
}

Daemon *Daemon::get_instance()
{
	// Create singleton object if first time
	if(instance == 0)
	{
		instance = new Daemon();
	}

	return instance;
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
	debugpp("waiting in recv_barrier_release" << exampi::rank);

	char msg[64];
	int err = ::recv(this->sock, msg, 64, NULL);

	debugpp("rank " << exampi::rank << " recv msg " << std::string(msg));

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
	return sendto(this->sock, packet.c_str(), packet.length(), NULL,
	              (sockaddr *)&this->daemon, sizeof(this->daemon));
}

std::string Daemon::recv()
{
	return std::string("");
}

}; // ::exampi
