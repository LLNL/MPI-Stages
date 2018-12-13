#include "debug.h"

#include <iomanip>
#include <thread>
#include <map>
#include <stack>

namespace exampi {

#ifdef DEBUG

std::mutex debug_mutex;
std::thread::id last_thread;
//std::thread::id main_thread_id = std::thread::get_id();

thread_local std::string thread_name("undef");
thread_local std::stack<std::string> thread_function_stack;

thread_local std::string last_file;
thread_local std::string last_function;
thread_local size_t func_depth = 0;

size_t thread_counter = 0;

void debug_add_thread(std::string name)
{
	std::stringstream fullname;

	fullname << "[thread ";

	fullname << std::setw(5);
	fullname << name;
	fullname << "] ";
	
	thread_name = fullname.str();
}

std::string debug_init(const char* file, int line, const char* func)
{
	std::string function(func);

	last_function = function;
	last_file = file;

	// check for main thread at init
	// TODO this should be in global space, that is the main
	// THIS HAS TO BE MOVED
	// TO GLOBAL IN DEBUG, this is the main thread
	if(thread_counter == 0)
	{
		thread_counter++;
		debug_add_thread("main");
		last_thread = std::this_thread::get_id();
	}

	std::stringstream stream;

	// indent appropriately
	for(size_t depth=0; depth < func_depth-1; ++depth)
		stream << std::string("\t\|");
	stream << std::string("\t");

	// check for new line needed on thread switch
	if(last_thread != std::this_thread::get_id()) {
		last_thread = std::this_thread::get_id();
	
		stream << "\n" << thread_name << " " << file;
		stream << "\n\t" << function;
		stream << "\n\t\t" << line << ": ";
	}
	else if(last_function.compare(function) != 0) {
		// this is the detection for level
		func_depth++;
		
		stream << "\t" << function;
		stream << "\n\t\t" << line << ": ";
	}
	else
	{
		stream << "\t\t" << line << ": ";
	}	
	
	//stream << std::string(thread_level, '\t') << thread_name;
	//stream << thread_name << "[" << file << ":" <<  line << "]\n\t[" << function << "] ";
	//stream << thread_name;
	
	//stream << line << ": ";

	return stream.str();
}

#endif

}
