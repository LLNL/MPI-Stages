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

#ifndef __EXAMPI_DEBUG_H
#define __EXAMPI_DEBUG_H

#include <mutex>
#include <string>
#include <iostream>

namespace exampi
{

#ifdef DEBUG
extern std::mutex debug_mutex;

// indent/thread/location info function for debug utility
std::string debug_init(const char *file, int line, const char *);

void debug_add_thread(std::string);

// are these useful?
// use automatic indentation somehow?
// these would restrict code structure
// exit specifically
void debug_function_entry(std::string);
void debug_function_exit();

#endif

#ifdef DEBUG

#define debug_add_thread(name) debug_add_thread(name)

#define debug(msg)	debug_mutex.lock(); \
                    std::clog << debug_init(__FILE__,__LINE__,__PRETTY_FUNCTION__) \
                    << msg << std::endl << std::flush; \
                    debug_mutex.unlock();

#define debug_error(msg)
#define debug_warning(msg)
#define debug_info(msg)
#define debug_info_deep(msg)
#define debug_time(msg)
#define debug_envelope(envelope)

#else

#define debug_add_thread(name)

#define debug_error(msg)
#define debug_warning(msg)
#define debug_info(msg)
#define debug_info_deep(msg)
#define debug_time(msg)

#define debug(msg)

#endif

}

#endif
