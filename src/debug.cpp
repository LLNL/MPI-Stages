#include "debug.h"

#include <iomanip>
#include <thread>
#include <map>
#include <stack>

namespace exampi {

#ifdef DEBUG

std::thread::id last_thread;
std::mutex debug_mutex;

thread_local std::string thread_name("undef");
thread_local size_t thread_level = 0;
thread_local std::stack<std::string> thread_function_stack;

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

void debug_function_entry(std::string name)
{
	thread_level++;
	thread_function_stack.push(name);
	
	// write to log
	debugpp("function entry: " << name);
}

void debug_function_exit()
{
	std::string name = thread_function_stack.top();
	thread_function_stack.pop();

	// write to log
	debugpp("function exit: " << name);

	thread_level--;
}

std::string debug_init()
{
	// check for main thread at init
	if(thread_counter == 0)
	{
		thread_counter++;
		debug_add_thread("main");
		last_thread = std::this_thread::get_id();
	}

	std::stringstream stream;

	// check for new line needed on thread switch
	if(last_thread != std::this_thread::get_id()) {
		last_thread = std::this_thread::get_id();
	
		stream << "\n";
	}
	
	stream << std::string(thread_level, '\t') << thread_name;

	return stream.str();
}

#endif

}
