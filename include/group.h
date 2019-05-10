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

#ifndef __EXAMPI_GROUP_H
#define __EXAMPI_GROUP_H

#include <list>

namespace exampi
{

class Group
{
public:
	Group();
	Group(std::list<int> processes);

	int get_group_id() const;
	void set_group_id(int id);

	std::list<int> get_process_list();
	void set_process_list(std::list<int> ranks);

private:
	std::list<int> process_list;

	int group_id;

	static int id;
};

}

#endif

