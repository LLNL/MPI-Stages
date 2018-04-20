#ifndef __EXAMPI_BASIC_INTERFACE_H
#define __EXAMPI_BASIC_INTERFACE_H

#include <basic.h>
#include <time.h>
#include <errHandler.h>
#include <funcType.h>
#include "basic/progress.h"

namespace exampi {
namespace basic {

class Interface: public exampi::i::Interface {
private:
	int rank;  // TODO: Don't keep this here, hand off to progress
	int ranks;
	std::vector<std::string> hosts;

public:
	Interface() :
		rank(0) {
	}
	;
	virtual int MPI_Init(int *argc, char ***argv) {

		// first param is config file, second is rank
		// third is epoch file, fourth is epoch
		std::cout << "Loading config from " << **argv << std::endl;
		exampi::global::config->load(**argv);
		exampi::global::worldSize = std::stoi(
				(*exampi::global::config)["size"]);
		(*argv)++;
		(*argc)--;
		std::cout << "Taking rank to be arg " << **argv << std::endl;
		rank = atoi(**argv);
		(*argv)++;
		(*argc)--;
		std::cout << "Taking epoch config to be " << **argv << std::endl;
		exampi::global::epochConfig = std::string(**argv);
		(*argv)++;
		(*argc)--;
		std::cout << "Taking epoch to be " << **argv << std::endl;
		exampi::global::epoch = atoi(**argv);
		(*argv)++;
		(*argc)--;

		exampi::global::rank = rank;
		int st = exampi::global::checkpoint->load();
#if 0
		exampi::global::rank = rank;
		exampi::global::transport->init();
		exampi::global::progress->init();
		//exampi::global::progress->barrier();
#endif
		if (exampi::global::epoch == 0)
			exampi::global::progress->barrier();

		std::cout << "Finished MPI_Init with code: " << st << "\n";
		return st;
	}

	virtual int MPI_Finalize() {
		return 0;
	}

	virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype,
			int dest, int tag, MPI_Comm comm) {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		size_t szcount = count;
		MPI_Status st = exampi::global::progress->postSend( {
			const_cast<void *>(buf), &(exampi::global::datatypes[datatype]),
					szcount }, { dest, comm }, tag).get();
		std::cout << debug() << "Finished MPI_Send: " << mpiStatusString(st)
								<< "\n";
		return 0;
	}

	virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
			int source, int tag, MPI_Comm comm, MPI_Status *status) {
		//exampi::global::progress->recv_data(buf, count, datatype, source, tag, comm, status);
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		size_t szcount = count;
		MPI_Status st = exampi::global::progress->postRecv( {
			const_cast<void *>(buf), &(exampi::global::datatypes[datatype]),
					szcount }, tag).get();
		std::cout << debug() << "Finished MPI_Recv: " << mpiStatusString(st)
								<< "\n";

		if (st.MPI_ERROR == MPIX_TRY_RELOAD) {
			handler.setErrToZero();
			memmove(status, &st, sizeof(MPI_Status));
			return MPIX_TRY_RELOAD;
		}
		memmove(status, &st, sizeof(MPI_Status));
		return 0;
	}

	virtual int MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
			int dest, int tag, MPI_Comm comm, MPI_Request *request) {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		size_t szcount = count;
		// have to move construct the future; i'll fix this later with a pool in progress
		std::future<MPI_Status> *f = new std::future<MPI_Status>();
		(*f) = exampi::global::progress->postSend( { const_cast<void *>(buf),
			&(exampi::global::datatypes[datatype]), szcount },
				{ dest, comm }, tag);
		(*request) = reinterpret_cast<MPI_Request>(f);

		return 0;
	}

	virtual int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
			int source, int tag, MPI_Comm comm, MPI_Request *request) {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		//exampi::global::progress->recv_data(buf, count, datatype, source, tag, comm, status);
		size_t szcount = count;
		std::future<MPI_Status> *f = new std::future<MPI_Status>();
		(*f) = exampi::global::progress->postRecv( { const_cast<void *>(buf),
			&(exampi::global::datatypes[datatype]), szcount }, {source, comm}, tag);
		(*request) = reinterpret_cast<MPI_Request>(f);
		return 0;
	}

	virtual int MPI_Wait(MPI_Request *request, MPI_Status *status) {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		std::future<MPI_Status> *f =
				reinterpret_cast<std::future<MPI_Status> *>(*request);
		(*status) = f->get();

		return MPI_SUCCESS;
	}

	virtual int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) {
		if (array_of_statuses) {
			for (int i = 0; i < count; i++) {
				if (array_of_request[i]) {
					array_of_statuses[i].MPI_ERROR = MPI_Wait(array_of_requests + i, array_of_statuses + i);
					if (array_of_statuses[i].MPI_ERROR)
						return -1;
				}
			}
		} else {
			for (int i = 0; i < count; i++) {
				int rc = MPI_Wait(array_of_requests + i, nullptr);
				if (rc)
					return rc;
			}
		}
		return MPI_SUCCESS;
	}

	virtual int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
			MPI_Comm comm) {
		if (exampi::global::rank == root) {
			for (int i = 0; i < exampi::global::worldSize; i++)
				if (i != root)
					MPI_Send(buf, count, datatype, i, 0, 0);
		} else {
			MPI_Status st;
			MPI_Recv(buf, count, datatype, root, 0, 0, &st);
		}
		return MPI_SUCCESS;
	}

	virtual int MPI_Comm_rank(MPI_Comm comm, int *r) {
		*r = rank;
		return 0;
	}

	virtual int MPI_Comm_size(MPI_Comm comm, int *r) {
		*r = std::stoi((*exampi::global::config)["size"]);
		return 0;
	}

	virtual int MPIX_Checkpoint() {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}
		exampi::global::checkpoint->save();
		return MPI_SUCCESS;
	}

	virtual int MPIX_Load_checkpoint() {
		sigHandler signal;
		signal.setSignalToHandle(SIGUSR1);
		int parent_pid = std::stoi((*exampi::global::config)["ppid"]);
		kill(parent_pid, SIGUSR1);

		while(signal.isSignalSet() != 1) {
			sleep(1);
		}

		std::ifstream ef(exampi::global::epochConfig);
		ef >> exampi::global::epoch;
		ef.close();

		return MPI_SUCCESS_RECOVERY;
	}

	virtual int MPIX_Get_fault_epoch(int *epoch) {
		errHandler handler;
		if (handler.isErrSet()) {
			handler.setErrToZero();
			return MPIX_TRY_RELOAD;
		}

		*epoch = exampi::global::epoch;
		return MPI_SUCCESS;
	}

	virtual int MPI_Barrier(MPI_Comm comm) {
		int rank, size;
		MPI_Status status;
		int coll_tag = 0;
		int rc;
		MPI_Comm_rank(comm, &rank);
		MPI_Comm_size(comm, &size);

		if (rank == 0) {
			rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
					comm, &status);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
					comm, &status);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
		} else {
			rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
					comm, &status);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
					comm, &status);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
			rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
			if (rc == MPIX_TRY_RELOAD) {
				return MPIX_TRY_RELOAD;
			}
		}
		return MPI_SUCCESS;
	}

	virtual double MPI_Wtime() {
		double wtime;
		struct timespec t ={.tv_sec = 0, .tv_nsec = 0};
		clock_gettime(CLOCK_REALTIME, &t);
		wtime = t.tv_sec;
		wtime += t.tv_nsec/1.0e+9;

		return wtime;
	}

	virtual int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err) {
		errHandler handler;
		handler.setErrToHandle(SIGUSR2);
		return MPI_SUCCESS;
	}

	virtual int MPI_Reduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm) {
		errHandler handler;
				if (handler.isErrSet()) {
					handler.setErrToZero();
					return MPIX_TRY_RELOAD;
				}
		int mask, comm_size, peer, peer_rank, peer_rel_rank, rc;
		int rank, rel_rank;

		MPI_Status status;
		Datatype *datatype;
		size_t bufsize;
		void *buf;

		if (count == 0) return MPI_SUCCESS;

		size_t szcount = count;
		datatype = &(exampi::global::datatypes[type]);
		bufsize = datatype->getExtent() * szcount;
		memcpy(r_buf, s_buf, bufsize);

		buf = malloc((size_t) bufsize);

		funcType::const_iterator iter = functions.find(op);

		rank = exampi::global::rank;
		comm_size = exampi::global::worldSize;
		if (comm_size == 1) return MPI_SUCCESS;

		rel_rank = (rank - root + comm_size) % comm_size;

		for (mask = 1; mask < comm_size; mask <<= 1) {
			peer_rel_rank = rel_rank ^ mask;
			if (peer_rel_rank >= comm_size) continue;
			peer_rank = (peer_rel_rank + root) % comm_size;
			if (peer_rank == root) {
				peer = root;
			}
			else {
				peer = peer_rank;
			}
			if (rel_rank < peer_rel_rank) {
				rc = MPI_Recv(buf, count, type, peer, 0, comm, &status);
				if (rc == MPIX_TRY_RELOAD) return rc;
				(*iter->second)(buf, r_buf, &count, &type);
			}
			else {
				usleep(500000);
				rc = MPI_Send(r_buf, count, type, peer, 0, comm);
				if (rc == MPIX_TRY_RELOAD) return rc;
				break;
			}
		}
		free(buf);
		return MPI_SUCCESS;
	}

	virtual int MPI_Allreduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, MPI_Comm comm) {
		errHandler handler;
				if (handler.isErrSet()) {
					handler.setErrToZero();
					return MPIX_TRY_RELOAD;
				}
		int rc = MPI_Reduce(s_buf, r_buf, count, type, op, 0, comm);
		if (rc != MPI_SUCCESS) {
			return rc;
		}
		rc = MPI_Bcast(r_buf, count, type, 0, comm);

		return rc;
	}

	virtual int MPI_Abort(MPI_Comm comm, int errorcode) {
		exit(-1);
		return errorcode;
	}
};

} // namespace basic
} // namespace exampi

#endif //...H
