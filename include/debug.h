#ifndef __EXAMPI_DEBUG_H
#define __EXAMPI_DEBUG_H

#include <mutex>
#include <string>
#include <iostream>
#include <sstream>

namespace exampi {

#ifdef DEBUG
extern std::mutex debug_mutex;

// indent/level/thread info function for debug utility
std::string debug_init();
void debug_add_thread(std::string);
void debug_function_entry(std::string);
void debug_function_exit();

#endif

// TODO rename this to debug once everything is wrapped, then sed to global replace
//#define debugpp(msg) std::clog << debug_init() << msg << std::endl;

#ifdef DEBUG

#define debug_add_thread(name) debug_add_thread(name)

#define debug_function_entry(name) debug_function_entry(name)  
#define debug_function_exit() debug_function_exit()

#define debugpp(msg) debug_mutex.lock(); std::clog << debug_init() << msg << std::endl << std::flush; debug_mutex.unlock();

#else

#define debug_add_thread(name)

#define debug_function_init(name) 
#define debug_function_exit() 

#define debugpp(msg)

#endif

}

#endif
