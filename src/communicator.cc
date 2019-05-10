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

#include "communicator.h"

#include "mpi.h"

namespace exampi
{

Comm::Comm()
{
}

Comm::Comm(bool _isintra, std::shared_ptr<Group> _local,
           std::shared_ptr<Group> _remote) :
	is_intra(_isintra), local(_local), remote(_remote)
{
}

Comm::~Comm()
{
}

// fix rule of 5 later
// Comm &operator=(Comm &rhs) = 0;
int Comm::get_next_context(int *pt2pt, int *coll)
{
	int rc;

	if (rank == 0)
	{
		//Context::contextLock.lock();

		Context::nextID++;
		*pt2pt = Context::nextID;
		Context::nextID++;
		*coll = Context::nextID;

		//Context::contextLock.unlock();
	}

	rc = MPI_Bcast(pt2pt, 1, MPI_INT, 0, local_pt2pt);
	if (rc != MPI_SUCCESS)
	{
		return MPIX_TRY_RELOAD;
	}

	rc = MPI_Bcast(coll, 1, MPI_INT, 0, local_pt2pt);
	if (rc != MPI_SUCCESS)
	{
		return MPIX_TRY_RELOAD;
	}

	return 0;
}

// accessors
std::shared_ptr<Group> Comm::get_local_group()
{
	return local;
}
std::shared_ptr<Group> Comm::get_remote_group()
{
	return remote;
}

void Comm::set_local_group(std::shared_ptr<Group> group)
{
	local = group;
}

void Comm::set_remote_group(std::shared_ptr<Group> group)
{
	remote = group;
}

int Comm::get_context_id_pt2pt() const
{
	return local_pt2pt;
}

int Comm::get_context_id_coll() const
{
	return local_coll;
}

void Comm::set_rank(int r)
{
	rank = r;
}

void Comm::set_context(int pt2pt, int coll)
{
	local_pt2pt = pt2pt;
	local_coll = coll;
}

int Comm::get_rank()
{
	return rank;
}

bool Comm::get_is_intra()
{
	return is_intra;
}

void Comm::set_is_intra(bool intra)
{
	is_intra = intra;
}

}
