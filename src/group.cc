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

#include <group.h>

namespace exampi
{

// todo this is a floating thing
int Group::id = 0;

Group::Group()
{
	;
}

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
