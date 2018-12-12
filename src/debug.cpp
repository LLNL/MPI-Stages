#include "debug.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <map>

namespace exampi {

std::string debug_init()
{
	std::stringstream stream;

	// TODO improve thread naming, hex is ugly and hard to read
	stream << "[0x" << std::hex << std::setfill('0') << std::setw(8) << std::this_thread::get_id() << "] ";

	// lookout into nice name table

	return stream.str();
}


std::mutex debug_mutex;

}

