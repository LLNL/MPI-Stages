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
	std::vector<MPIX_Serialize_handler> serialize_handlers;
	std::vector<MPIX_Deserialize_handler> deserialize_handlers;
	int recovery_code;
public:
	Interface() : rank(0) {}

	/*
	 * 1st param: config file
	 * 2nd param: rank
	 * 3rd param: epoch file
	 * 4th param: epoch
	 */
	virtual int MPI_Init(int *argc, char ***argv) {
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
		recovery_code = exampi::global::checkpoint->load();

		if (exampi::global::epoch == 0)
			exampi::global::progress->barrier();

		/* Checkpoint/restart
		 * errHandler handler;
		 * handler.setErrToHandle(SIGUSR2);
		 */

		std::cout << "Finished MPI_Init with code: " << recovery_code << "\n";
		return recovery_code;
	}

	virtual int MPI_Finalize() {
		serialize_handlers.clear();
		deserialize_handlers.clear();
		exampi::global::transport->finalize();
		exampi::global::progress->finalize();
		return 0;
	}

	virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype,
			int dest, int tag, MPI_Comm comm) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);
		int context = c->get_context_id_pt2pt();
		size_t szcount = count;
		MPI_Status st = exampi::global::progress->postSend( {
			const_cast<void *>(buf), &(exampi::global::datatypes[datatype]),
					szcount }, { dest, context }, tag).get();
		std::cout << debug() << "Finished MPI_Send: " << mpiStatusString(st)
								<< "\n";
		return 0;
	}

	virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
			int source, int tag, MPI_Comm comm, MPI_Status *status) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);
		int context = c->get_context_id_pt2pt();
		size_t szcount = count;
		MPI_Status st = exampi::global::progress->postRecv( {
			const_cast<void *>(buf), &(exampi::global::datatypes[datatype]),
					szcount }, {source, context}, tag).get();
		std::cout << debug() << "Finished MPI_Recv: " << mpiStatusString(st)
								<< "\n";

		if (st.MPI_ERROR == MPIX_TRY_RELOAD) {
			memmove(status, &st, sizeof(MPI_Status));
			return MPIX_TRY_RELOAD;
		}
		memmove(status, &st, sizeof(MPI_Status));
		return 0;
	}

	virtual int MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
			int dest, int tag, MPI_Comm comm, MPI_Request *request) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);
		int context = c->get_context_id_pt2pt();
		size_t szcount = count;
		// have to move construct the future; i'll fix this later with a pool in progress
		std::future<MPI_Status> *f = new std::future<MPI_Status>();
		(*f) = exampi::global::progress->postSend( { const_cast<void *>(buf),
			&(exampi::global::datatypes[datatype]), szcount },
				{ dest, context }, tag);
		(*request) = reinterpret_cast<MPI_Request>(f);

		return 0;
	}

	virtual int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
			int source, int tag, MPI_Comm comm, MPI_Request *request) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);
		int context = c->get_context_id_pt2pt();

		size_t szcount = count;
		std::future<MPI_Status> *f = new std::future<MPI_Status>();
		(*f) = exampi::global::progress->postRecv( { const_cast<void *>(buf),
			&(exampi::global::datatypes[datatype]), szcount }, {source, context}, tag);
		(*request) = reinterpret_cast<MPI_Request>(f);
		return 0;
	}

	virtual int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		MPI_Request recvreq;
		int rc = MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm, &recvreq);
		if (rc != MPI_SUCCESS) {
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);
		if (rc != MPI_SUCCESS) {
			return MPIX_TRY_RELOAD;
		}
		MPI_Wait(&recvreq, status);
		return MPI_SUCCESS;
	}

	virtual int MPI_Wait(MPI_Request *request, MPI_Status *status) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		std::future<MPI_Status> *f =
				reinterpret_cast<std::future<MPI_Status> *>(*request);
		(*status) = f->get();

		return MPI_SUCCESS;
	}

	virtual int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		if (array_of_statuses) {
			for (int i = 0; i < count; i++) {
				if (array_of_requests[i]) {
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
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		int rc;
		if (exampi::global::rank == root) {
			for (int i = 0; i < exampi::global::worldSize; i++)
				if (i != root) {
					rc = MPI_Send(buf, count, datatype, i, 0, comm);
					if (rc != MPI_SUCCESS)
						return MPIX_TRY_RELOAD;
				}
		} else {
			MPI_Status st;
			rc = MPI_Recv(buf, count, datatype, root, 0, comm, &st);
			if (rc != MPI_SUCCESS)
				return MPIX_TRY_RELOAD;
		}
		return MPI_SUCCESS;
	}

	virtual int MPI_Comm_rank(MPI_Comm comm, int *r) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);

		*r = (c)->get_rank();
		return 0;
	}

	virtual int MPI_Comm_size(MPI_Comm comm, int *r) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		Comm *c = exampi::global::communicators.at(comm);

		*r = (c)->get_local_group()->get_process_list().size();
		return 0;
	}

	virtual int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		int rc;
		Comm *c = exampi::global::communicators.at(comm);

		int coll_context;
		int p2p_context;
		rc = (c)->get_next_context(&p2p_context, &coll_context);
		if (rc != MPI_SUCCESS) {
			return MPIX_TRY_RELOAD;
		}
		Comm *communicator;
		communicator = new Comm(true, (c)->get_local_group(), (c)->get_remote_group());
		communicator->set_rank((c)->get_rank());
		communicator->set_context(p2p_context, coll_context);
		exampi::global::communicators.push_back(communicator);
		auto it = std::find_if(exampi::global::communicators.begin(), exampi::global::communicators.end(),
											[communicator](const Comm *i) -> bool {return i->get_context_id_pt2pt() == communicator->get_context_id_pt2pt();});
		if (it == exampi::global::communicators.end()) {
			return MPIX_TRY_RELOAD;
		}
		else {
			*newcomm = std::distance(exampi::global::communicators.begin(), it);
		}
		return MPI_SUCCESS;
	}

	virtual int MPIX_Serialize_handles() {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}

		std::stringstream filename;
		filename << exampi::global::epoch - 1 << "." << exampi::global::rank << ".cp";
		std::ofstream t(filename.str(), std::ofstream::out | std::ofstream::app);

		if (!serialize_handlers.empty()) {
			for (auto it : serialize_handlers) {
				MPIX_Handles handles;
				(*it)(&handles);
				t.write(reinterpret_cast<char *>(&handles.comm_size), sizeof(int));
				for (int i = 0; i < handles.comm_size; i++) {
					Comm *c = exampi::global::communicators.at(handles.comms[i]);
					int id = c->get_context_id_pt2pt();
					t.write(reinterpret_cast<char *>(&id), sizeof(int));
				}

				t.write(reinterpret_cast<char *>(&handles.group_size), sizeof(int));
				for (int i = 0; i < handles.group_size; i++) {
					Group *g = exampi::global::groups.at(handles.grps[i]);
					int id = g->get_group_id();
					t.write(reinterpret_cast<char *>(&id), sizeof(int));
				}
			}
		}
		t.close();

		return MPI_SUCCESS;
	}

	virtual int MPIX_Deserialize_handles() {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		std::stringstream filename;
		filename << exampi::global::epoch - 1 << "." << exampi::global::rank << ".cp";
	    std::ifstream t(filename.str(), std::ifstream::in);

	    long long int pos;
	    t.read(reinterpret_cast<char *>(&pos), sizeof(long long int));
	    t.seekg(pos);

	    int size, id;

	    if (!deserialize_handlers.empty()) {
	    	for (auto iter : deserialize_handlers) {
	    		MPIX_Handles handles;
	    		t.read(reinterpret_cast<char *>(&size), sizeof(int));
	    		handles.comm_size = size;
	    		if (handles.comm_size > 0) {
	    			handles.comms = (MPI_Comm *)malloc(size * sizeof(MPI_Comm));
	    			for (int i = 0; i < size; i++) {
	    				t.read(reinterpret_cast<char *>(&id), sizeof(int));
	    				auto it = std::find_if(exampi::global::communicators.begin(), exampi::global::communicators.end(),
	    											[id](const Comm *i) -> bool {return i->get_context_id_pt2pt() == id;});
	    				if (it == exampi::global::communicators.end()) {
	    					return MPIX_TRY_RELOAD;
	    				}
	    				else {
	    					handles.comms[i] = std::distance(exampi::global::communicators.begin(), it);
	    				}
	    			}
	    		}
	    		t.read(reinterpret_cast<char *>(&size), sizeof(int));
	    		handles.group_size = size;
	    		if (handles.group_size > 0) {
	    			handles.grps = (MPI_Group *)malloc(size * sizeof(MPI_Group));
	    			for (int i = 0; i < size; i++) {
	    				t.read(reinterpret_cast<char *>(&id), sizeof(int));
	    				auto it = std::find_if(exampi::global::groups.begin(), exampi::global::groups.end(),
	    			    							[id](const Group *i) -> bool {return i->get_group_id() == id;});
	    				if (it == exampi::global::groups.end()) {
	    					return MPIX_TRY_RELOAD;
	    				}
	    				else {
	    					handles.grps[i] = std::distance(exampi::global::groups.begin(), it);
	    				}
	    			}
	    		}
	    		(*iter)(handles);
	    	}
	    }

	    t.close();
		return MPI_SUCCESS;
	}

	virtual int MPIX_Serialize_handler_register(const MPIX_Serialize_handler handler) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		if (exampi::global::epoch == 0 && recovery_code == MPI_SUCCESS) {
			serialize_handlers.push_back(handler);
		}
		else if (exampi::global::epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART) {
			serialize_handlers.push_back(handler);
		}

		return MPI_SUCCESS;
	}

	virtual int MPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		if (exampi::global::epoch == 0 && recovery_code == MPI_SUCCESS) {
			deserialize_handlers.push_back(handler);
		}
		else if (exampi::global::epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART) {
			deserialize_handlers.push_back(handler);
		}

		return MPI_SUCCESS;
	}

	virtual int MPIX_Checkpoint_write() {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		exampi::global::checkpoint->save();
		return MPI_SUCCESS;
	}

	virtual int MPIX_Checkpoint_read() {
		if (exampi::global::handler->isErrSet()) {
			exampi::global::handler->setErrToZero();
		}
		sigHandler signal;

		while(signal.isSignalSet() != 1) {
			sleep(1);
		}

		std::ifstream ef(exampi::global::epochConfig);
		ef >> exampi::global::epoch;
		ef.close();

		return MPI_SUCCESS;
	}

	virtual int MPIX_Get_fault_epoch(int *epoch) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}

		*epoch = exampi::global::epoch;
		return MPI_SUCCESS;
	}

	virtual int MPI_Barrier(MPI_Comm comm) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
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
		exampi::global::handler->setErrToHandle(SIGUSR2);
		return MPI_SUCCESS;
	}

	virtual int MPI_Reduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm) {
		if (exampi::global::handler->isErrSet()) {
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
				rc = MPI_Send(r_buf, count, type, peer, 0, comm);
				if (rc == MPIX_TRY_RELOAD) return rc;
				break;
			}
		}
		free(buf);
		return MPI_SUCCESS;
	}

	virtual int MPI_Allreduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, MPI_Comm comm) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		int rc = MPI_Reduce(s_buf, r_buf, count, type, op, 0, comm);
		if (rc != MPI_SUCCESS) {
			return rc;
		}
		rc = MPI_Bcast(r_buf, count, type, 0, comm);

		return rc;
	}

	virtual int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count) {
		if (exampi::global::handler->isErrSet()) {
			return MPIX_TRY_RELOAD;
		}
		exampi::Datatype type = exampi::global::datatypes[datatype];
		if (type.getExtent()) {
			*count = status->count / type.getExtent();
			if (status->count >= 0 && *count * type.getExtent() != static_cast<size_t>(status->count)) {
				*count = MPI_UNDEFINED;
			}
		}
		else if (status->count == 0) {
			*count = 0;
		}
		else {
			*count = MPI_UNDEFINED;
		}
		return MPI_SUCCESS;
	}

	virtual int MPI_Abort(MPI_Comm comm, int errorcode) {
		exit(-1);
		return errorcode;
	}
};

} // namespace basic
} // namespace exampi

#endif //...H
