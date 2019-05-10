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

#include <iostream>

#include "faulthandler.h"
#include "universe.h"
#include "daemon.h"
//#include "engines/blockingprogress.h"

namespace exampi
{

volatile std::sig_atomic_t FaultHandler::is_errSet = 0;

FaultHandler &FaultHandler::get_instance()
{
	static FaultHandler faulthandler;

	return faulthandler;
}

bool FaultHandler::setErrToHandle(int sig)
{
	return (std::signal(sig, setErr) != SIG_ERR);
}

void FaultHandler::setErr(int unused)
{
	is_errSet = 1;

	Daemon &daemon = Daemon::get_instance();
	daemon.send_clean_up();

	Universe &universe = Universe::get_root_universe();
	universe.progress->cleanup();
}

void FaultHandler::setErrToZero()
{
	is_errSet = 0;
}

void FaultHandler::setErrToOne()
{
	is_errSet = 1;
}

int FaultHandler::isErrSet()
{
	return is_errSet;
}

}
