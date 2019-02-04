#include "debug.h"

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
	// TODO this should be in global space, that is the main
	// THIS HAS TO BE MOVED
	// TO GLOBAL IN DEBUG, this is the main thread
	//if(thread_counter == 0)
	//{
	//	thread_counter++;
	//	debug_add_thread("main");
	//	last_thread = std::this_thread::get_id();
	//}

	std::stringstream stream;

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
	// TODO broken at the moment

	if(last_function.empty())
	{
		last_function = function;
		stream << "\t[" << function << "]" << std::endl;
	}
	else if(last_function.compare(function) != 0)
	{
		// either decrement or increment
		// NOTE does not support recursion
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

	return stream.str();
}

#endif

}
