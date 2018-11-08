#ifndef __EXAMPI_DAEMON_H
#define __EXAMPI_DAEMON_H

#include <string>
#include <netinet/in.h>

namespace exampi
{

class Daemon
{
public:
	static Daemon *get_instance();

	int send_barrier_ready();
	int recv_barrier_release();
	int send_clean_up();

private:
	static Daemon *instance;

	int sock;
	sockaddr_in daemon;
	sockaddr_in local;

	Daemon();
	~Daemon();

	int send(std::string);
	std::string recv();
};

} // ::exampi

#endif // guard
