#ifndef __EXAMPI_SIGHANDLER_H
#define __EXAMPI_SIGHANDLER_H

#include <csignal>

namespace exampi
{
class sigHandler
{
public:
	sigHandler();
	~sigHandler();

	bool setSignalToHandle(int sig);
	static int isSignalSet();
	static void setSignal();
	static void setSignalToZero();

private:
	volatile static std::sig_atomic_t is_signalSet;
};
}
#endif
