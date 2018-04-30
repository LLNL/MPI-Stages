#ifndef __EXAMPI_GROUP_H
#define __EXAMPI_GROUP_H

#include <list>

namespace exampi {

class Group
{
 public:
	Group() {};
	Group(std::list<int> processes) : process_list(processes) {}
	std::list<int> getProcessList() {
		return process_list;
	}
 protected:
	std::list<int> process_list;
 private:

};

}

#endif

