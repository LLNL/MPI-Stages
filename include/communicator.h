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

#ifndef __EXAMPI_COMM_H
#define __EXAMPI_COMM_H

#include <context.h>
#include <group.h>
#include <memory>

namespace exampi
{

struct Comm
{
	Comm();
	Comm(bool _isintra, std::shared_ptr<Group> _local,
	     std::shared_ptr<Group> _remote);

	virtual ~Comm();

	// fix rule of 5 later
	// Comm &operator=(Comm &rhs) = 0;
	int get_next_context(int *pt2pt, int *coll);

	// accessors
	std::shared_ptr<Group> get_local_group();
	std::shared_ptr<Group> get_remote_group();

	void set_local_group(std::shared_ptr<Group> group);
	void set_remote_group(std::shared_ptr<Group> group);

	int get_context_id_pt2pt() const;
	int get_context_id_coll() const;

	void set_rank(int r);
	void set_context(int pt2pt, int coll);

	int get_rank();

	bool get_is_intra();
	void set_is_intra(bool intra);

	// Nawrin task for later, introduce the entire MPI API here as methods of this comm; right now, we do "Shane-mode,"
	// where the C API directly calls the Interface singleton, which is allowed to use accessors of Comm for info.
	//
	// [future version only]
	//
	bool is_intra;
	std::shared_ptr<Group> local;
	std::shared_ptr<Group> remote;

	int local_pt2pt;
	int local_coll;

	int rank;

	//  context_id remote_pt2pt;
	//  context_id remote_coll;

};

}

#endif
