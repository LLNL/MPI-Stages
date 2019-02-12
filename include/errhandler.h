#ifndef __EXAMPI_ERRHANDLER_H
#define __EXAMPI_ERRHANDLER_H

#include <csignal>

namespace exampi
{

class errHandler
{
public:
	static errHandler &get_instance();

	errHandler(const errHandler &e)				= delete;
	errHandler &operator=(const errHandler &e)	= delete;
	
	bool setErrToHandle(int sig);

	static int isErrSet();
	static void setErr(int unused);
	static void setErrToZero();
	static void setErrToOne();

private:
	errHandler() {}

	volatile static std::sig_atomic_t is_errSet;
};

}

#endif
