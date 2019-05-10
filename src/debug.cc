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

#include "debug.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <map>
#include <stack>

namespace exampi
{

#ifdef DEBUG

std::mutex debug_mutex;
std::thread::id last_thread = std::this_thread::get_id();

std::thread::id main_thread = std::this_thread::get_id();

thread_local std::string thread_name("[thread undef]");

thread_local std::string last_file;
thread_local std::string last_function;
thread_local std::stack<std::string> function_stack;
thread_local size_t func_depth = 0;

void debug_add_thread(std::string name)
{
	std::stringstream fullname;

	fullname << "[thread ";

	fullname << std::setw(5);
	fullname << name;
	fullname << "] ";

	thread_name = fullname.str();
}

std::string debug_init(const char *file, int line, const char *func)
{
	// name main thread
	if(std::this_thread::get_id() == main_thread)
		debug_add_thread("main");

	std::string function(func);

	// check for main thread at init
	//if(thread_counter == 0)
	//{
	//	thread_counter++;
	//	debug_add_thread("main");
	//	last_thread = std::this_thread::get_id();
	//}

	std::stringstream stream;

	// todo prepend symbol string for each thread, easier than text name
	// ####
	// ****
	// &&&&
	// @@@@
	// %%%%
	// !!!!
	// ????
	// todo register threads that we create, progress threads for example
	// ## 01 internal
	// ## 02 internal
	// ## 03 internal
	// ## 04 internal
	// $$ 01 external
	// $$ 02 external
	// $$ 03 external


	// check for new line needed on thread switch
	if(last_thread != std::this_thread::get_id())
	{
		last_thread = std::this_thread::get_id();

		stream << "\n" << thread_name << " [" << file << "]" << std::endl;
	}

	// indent appropriately
	//for(size_t depth = 0; depth < func_depth; ++depth)
	//{
	//	stream << std::string("\t");
	//}
	// todo broken at the moment

	if(last_function.empty())
	{
		last_function = function;
		stream << "\t[" << function << "]" << std::endl;
	}
	else if(last_function.compare(function) != 0)
	{
		// either decrement or increment
		// note does not support recursion
		if((function_stack.size() > 0) && (function_stack.top().compare(function) == 0))
		{
			// returned
			last_function = function_stack.top();
			function_stack.pop();
			func_depth--;
		}
		else
		{
			// called next
			function_stack.push(last_function);
			last_function = function;
			func_depth++;
		}

		stream << "\t[" << function << "]" << std::endl;
	}

	stream << "\t\t" << func_depth << " " << std::setw(4) << line << ": ";
	stream << "\t\t" << std::setw(4) << line << ": ";

	return stream.str();
}

#endif

}
