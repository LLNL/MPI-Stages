#include "daemon.h"
#include "global.h"

#include <sstream>

namespace exampi
{

Daemon *Daemon::instance = nullptr;

std::string UDP_IP("127.0.0.1");
int BASE_UDP_PORT = 50000;

Daemon::Daemon()
{
	// initiate socket
	this->sock = socket(AF_INET, SOCK_DGRAM, 0);

	//
	this->destination.sin_family = AF_INET;
	this->destination.sin_port = BASE_UDP_PORT;

	inet_aton(UDP_IP.c_str(), &this->destination.sin_addr);
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
	packet << exampi::rank << " ";
	packet << "barrier ";
	packet << getpid();

	return send(packet.str());
}

int Daemon::recv_barrier_release()
{
	return -123234;
}

int Daemon::send_clean_up()
{
	std::stringstream packet;
	packet << exampi::rank << " ";
	packet << getpid() << " ";
	packet << exampi::epoch;

	return send(packet.str());
}

int Daemon::send(std::string packet)
{
	// check if socket is open
	if(this->sock < 0)
		return this->sock;
	else if(packet.length() > 64)
		return -545634;

	// send packet to daemon
	return sendto(this->sock, packet.c_str(), packet.length(), NULL,
	              (sockaddr *)&this->destination, sizeof(this->destination));
}

std::string Daemon::recv()
{
	return std::string("");
}

}; // ::exampi
