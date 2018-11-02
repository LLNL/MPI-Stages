#ifndef __EXAMPI_GROUP_H
#define __EXAMPI_GROUP_H

#include <list>

namespace exampi {

class Group
{
 public:
	Group();
	Group(std::list<int> processes);
	std::list<int> get_process_list();
	int get_group_id() const;
	void set_group_id(int id);
	void set_process_list(std::list<int> ranks);

 protected:
 private:
	std::list<int> process_list;
	int group_id;
	static int id;
};

}

#endif

