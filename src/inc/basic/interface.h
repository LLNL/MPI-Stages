#ifndef __EXAMPI_BASIC_INTERFACE_H
#define __EXAMPI_BASIC_INTERFACE_H

#include <basic.h>
#include "basic/progress.h"

namespace exampi {
namespace basic {

class Interface : public exampi::i::Interface
{
  private:
    int rank;  // TODO: Don't keep this here, hand off to progress
    int ranks;
    std::vector<std::string> hosts;

  public:
    Interface() : rank(0) {};
    virtual int MPI_Init(int *argc, char ***argv)
    {

      // first param is config file, second is rank
      std::cout << "Loading config from " << **argv << std::endl;
      exampi::global::config->load(**argv);
      (*argv)++;
      (*argc)--;
      std::cout << "Taking rank to be arg " << **argv << std::endl;
      rank = atoi(**argv);
      (*argv)++;
      (*argc)--;

      exampi::global::rank = rank;
      exampi::global::worldSize = std::stoi((*exampi::global::config)["size"]);
      exampi::global::transport->init();
      exampi::global::progress->init();
      //exampi::global::progress->barrier();
      std::cout << "Finished MPI_Init\n";
      return 0;
    }
    virtual int MPI_Finalize() { return 0; }

    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      //exampi::global::progress->send_data(const_cast<void *>(buf), static_cast<size_t>(count), datatype, dest, tag, comm);
      size_t szcount = count;
      MPI_Status st = exampi::global::progress->postSend(
          {const_cast<void *>(buf), &(exampi::global::datatypes[datatype]), szcount},
          {dest, comm},
          tag).get();
      std::cout << debug() << "Finished MPI_Send: " << mpiStatusString(st) << "\n";
    	return 0;
    }

    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      //exampi::global::progress->recv_data(buf, count, datatype, source, tag, comm, status);
      size_t szcount = count;
      MPI_Status st = exampi::global::progress->postRecv(
          {const_cast<void *>(buf), &(exampi::global::datatypes[datatype]), szcount},
          tag).get();
      std::cout << debug() << "Finished MPI_Recv: " << mpiStatusString(st) << "\n";
      memmove(status, &st, sizeof(MPI_Status));
    	return 0;
    }

    virtual int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
    {
      size_t szcount = count;
      // have to move construct the future; i'll fix this later with a pool in progress
      std::future<MPI_Status> *f = new std::future<MPI_Status>();
      (*f) = exampi::global::progress->postSend(
            {const_cast<void *>(buf), &(exampi::global::datatypes[datatype]), szcount},
            {dest, comm}, tag);
      (*request) = reinterpret_cast<MPI_Request>(f);    
    
      return 0;
    }

    virtual int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) {
      //exampi::global::progress->recv_data(buf, count, datatype, source, tag, comm, status);
      size_t szcount = count;
      std::future<MPI_Status> *f = new std::future<MPI_Status>();
      (*f) = exampi::global::progress->postRecv(
            {const_cast<void *>(buf), &(exampi::global::datatypes[datatype]), szcount},
            tag);
      (*request) = reinterpret_cast<MPI_Request>(f);
    	return 0;
    }

    virtual int MPI_Wait(MPI_Request *request, MPI_Status *status)
    {
      std::future<MPI_Status> *f = reinterpret_cast<std::future<MPI_Status> *>(*request);
      (*status) = f->get();
      return 0;
    }

    virtual int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
    {
      if(exampi::global::rank == root)
      {
        for(int i = 0; i < exampi::global::worldSize; i++)
          if(i != root) 
            MPI_Send(buf, count, datatype, i, 0, 0);
      }
      else
      {
        MPI_Status st;
        MPI_Recv(buf, count, datatype, root, 0, 0, &st);
      }          
    }

    virtual int MPI_Comm_rank(MPI_Comm comm, int *r)
    {
      *r = rank;
      return 0;
    }

    virtual int MPI_Comm_size(MPI_Comm comm, int *r)
    {
      *r = std::stoi((*exampi::global::config)["size"]);
      return 0;
    }
};


} // namespace basic
} // namespace exampi

#endif //...H
