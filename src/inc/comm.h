#ifndef __EXAMPI_COMM_H
#define __EXAMPI_COMM_H

#include <global.h>
#include <context.h>
#include <group.h>
#include <memory>

namespace exampi {

class Comm {
public:
	Comm() {
	}
	Comm(bool _isintra, std::shared_ptr<Group> _local,
			std::shared_ptr<Group> _remote) :
			isintra(_isintra), local(_local), remote(_remote) {
	}

	virtual ~Comm() {
	}

	// fix rule of 5 later
	// Comm &operator=(Comm &rhs) = 0;
	int get_next_context(int *pt2pt, int *coll) {
		int rc;
		if (rank == 0) {
			//Context::contextLock.lock();
			Context::nextID++;
			*pt2pt = Context::nextID;
			Context::nextID++;
			*coll = Context::nextID;
			//Context::contextLock.unlock();
		}
		rc = MPI_Bcast(pt2pt, 1, MPI_INT, 0, local_pt2pt);
		if (rc != MPI_SUCCESS) {
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Bcast(coll, 1, MPI_INT, 0, local_pt2pt);
		if (rc != MPI_SUCCESS) {
			return MPIX_TRY_RELOAD;
		}

		return 0;
	}
	// accessors
	std::shared_ptr<Group> get_local_group() {
		return local;
	}
	std::shared_ptr<Group> get_remote_group() {
		return remote;
	}
	const int get_context_id_pt2pt() {
		return local_pt2pt;
	}
	const int get_context_id_coll() {
		return local_coll;
	}
	void setRank(int r) {
		rank = r;
	}
	void set_context(int pt2pt, int coll) {
		local_pt2pt = pt2pt;
		local_coll = coll;
	}

	int getRank() {
		return rank;
	}

	// Nawrin task for later, introduce the entire MPI API here as methods of this comm; right now, we do "Shane-mode,"
	// where the C API directly calls the Interface singledon, which is allowed to use accessors of Comm for info.
	//
	// [future version only]
	//
protected:
	bool isintra;
	std::shared_ptr<Group> local;
	std::shared_ptr<Group> remote;

	int local_pt2pt;
	int local_coll;

	int rank;

	//  context_id remote_pt2pt;
	//  context_id remote_coll;

private:

};

}

#endif
