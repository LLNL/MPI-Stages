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

#ifndef __EXAMPI_FAULTHANDLER_H
#define __EXAMPI_FAULTHANDLER_H

#include <csignal>

namespace exampi
{

class FaultHandler
{
public:
	static FaultHandler &get_instance();

	FaultHandler(const FaultHandler &e)				= delete;
	FaultHandler &operator=(const FaultHandler &e)	= delete;

	bool setErrToHandle(int sig);

	static int isErrSet();
	static void setErr(int unused);
	static void setErrToZero();
	static void setErrToOne();

private:
	FaultHandler() {}

	volatile static std::sig_atomic_t is_errSet;
};

}

#endif
