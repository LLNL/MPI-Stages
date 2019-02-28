#include <iostream>

#include "faulthandler.h"
#include "universe.h"

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

	//Universe &universe = Universe::get_root_universe();
	//universe.halt();
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
