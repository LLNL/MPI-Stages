#include <sigHandler.h>

namespace exampi
{

volatile std::sig_atomic_t sigHandler::is_signalSet = 0;

sigHandler::sigHandler() {}

sigHandler::~sigHandler() {}

bool sigHandler::setSignalToHandle(int sig)
{
	if(std::signal(sig, sigHandler::setSignal) == SIG_ERR)
	{
		return false;
	}
	return true;
}

void sigHandler::setSignal(int unused)
{
	is_signalSet = 1;
}

void sigHandler::setSignalToZero()
{
	is_signalSet = 0;
}

int sigHandler::isSignalSet()
{
	return is_signalSet;
}

}
