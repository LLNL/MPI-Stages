#ifndef __EXAMPI_DEBUG_H
#define __EXAMPI_DEBUG_H

#include <mutex>
#include <string>

namespace exampi {

// debug print mutex, so threads don't overwrite each others output
// this makes debug really slow
extern std::mutex debug_mutex;

// indent/level/thread info function for debug utility
std::string debug_init();

// TODO rename this to debug once everything is wrapped, then sed to global replace
//#define debugpp(msg) std::clog << debug_init() << msg << std::endl;

#ifdef DEBUG

#define debug_function_init(name) 
#define debug_function_exit(name) 

#define debugpp(msg) debug_mutex.lock(); std::clog << debug_init() << msg << std::endl << std::flush; debug_mutex.unlock();

#else

#define debugpp(msg)

#endif

}

#endif
