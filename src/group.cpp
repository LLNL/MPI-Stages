#include <group.h>

namespace exampi
{

int Group::id = 0;

Group::Group() {};

Group::Group(std::list<int> processes) : process_list(processes),
	group_id(id++) {}

std::list<int> Group::get_process_list()
{
	return process_list;
}

int Group::get_group_id() const
{
	return group_id;
}

void Group::set_group_id(int id)
{
	group_id = id;
}

void Group::set_process_list(std::list<int> ranks)
{
	process_list = ranks;
}

}
