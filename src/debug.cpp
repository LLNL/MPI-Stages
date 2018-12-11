#include "debug.h"

#include <iostream>

namespace exampi {

// TODO refactor/redo this indentation depending on thread and level?
// level would be very nice, T3DMPI
std::string debut_init()
{
	return std::string("");
}
//{
//	std::stringstream stream;
//
//	//debug_stringstream << "\t[0x" << std::hex << std::setfill('0') << std::setw(
//	//                       8) << thisThread() << "] ";
//
//	return stream.str();
//}


std::mutex debug_mutex;

}

