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

#ifndef __EXAMPI_DAEMON_H
#define __EXAMPI_DAEMON_H

#include <string>
#include <netinet/in.h>

namespace exampi
{

class Daemon
{
public:
	static Daemon &get_instance();

	Daemon(Daemon &)				= delete;
	void operator=(Daemon const &)	= delete;

	int barrier();
	int send_clean_up();
	int wait_commit();
	int abort();

private:
	int send_barrier_ready();
	int recv_barrier_release();

	int sock;
	sockaddr_in daemon;

	Daemon();
	~Daemon();

	int send(std::string);
	std::string recv();
};

} // ::exampi

#endif // guard
