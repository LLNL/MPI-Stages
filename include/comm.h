#ifndef __EXAMPI_COMM_H
#define __EXAMPI_COMM_H

#include <context.h>
#include <group.h>
#include <memory>

namespace exampi
{

struct Comm
{
	Comm()
	{
	}
	Comm(bool _isintra, std::shared_ptr<Group> _local, std::shared_ptr<Group> _remote) :
		is_intra(_isintra), local(_local), remote(_remote)
	{
	}

	virtual ~Comm()
	{
	}

	// fix rule of 5 later
	// Comm &operator=(Comm &rhs) = 0;
	int get_next_context(int *pt2pt, int *coll)
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
	std::shared_ptr<Group> get_local_group()
	{
		return local;
	}
	std::shared_ptr<Group> get_remote_group()
	{
		return remote;
	}

	void set_local_group(std::shared_ptr<Group> group)
	{
		local = group;
	}

	void set_remote_group(std::shared_ptr<Group> group)
	{
		remote = group;
	}

	int get_context_id_pt2pt() const
	{
		return local_pt2pt;
	}

	int get_context_id_coll() const
	{
		return local_coll;
	}

	void set_rank(int r)
	{
		rank = r;
	}

	void set_context(int pt2pt, int coll)
	{
		local_pt2pt = pt2pt;
		local_coll = coll;
	}

	int get_rank()
	{
		return rank;
	}

	bool get_is_intra()
	{
		return is_intra;
	}

	void set_is_intra(bool intra)
	{
		is_intra = intra;
	}

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
