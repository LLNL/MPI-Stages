#include <iostream>

#include "errhandler.h"
#include "universe.h"

namespace exampi
{

volatile std::sig_atomic_t errHandler::is_errSet = 0;

errHandler &get_instance()
{
	static errHandler errhandler;

	return errhandler;
}

bool errHandler::setErrToHandle(int sig)
{
	return (std::signal(sig, setErr) != SIG_ERR);
}

void errHandler::setErr(int unused)
{
	is_errSet = 1;

	Universe& universe = Universe::get_root_universe();
	universe.progress->cleanUp();
}

void errHandler::setErrToZero()
{
	is_errSet = 0;
}

void errHandler::setErrToOne()
{
	is_errSet = 1;
}

int errHandler::isErrSet()
{
	return is_errSet;
}

}
