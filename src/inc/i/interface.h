#ifndef __EXAMPI_I_INTERFACE_H
#define __EXAMPI_I_INTERFACE_H

#include <mpi.h>

namespace exampi {
namespace i {

class Interface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
    virtual int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;
    virtual int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;
    virtual int MPI_Wait(MPI_Request *request, MPI_Status *status) = 0;
    virtual int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) = 0;
    virtual int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) = 0;
    virtual int MPI_Comm_rank(MPI_Comm comm, int *r) = 0;
    virtual int MPI_Comm_size(MPI_Comm comm, int *r) = 0;
    virtual int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) = 0;
    virtual int MPIX_Serialize(MPI_Comm comm) = 0;
    virtual int MPIX_Deserialize(MPI_Comm *comm) = 0;

    virtual int MPIX_Checkpoint(void) = 0;
    virtual int MPIX_Load_checkpoint(void) = 0;
    virtual int MPIX_Get_fault_epoch(int *epoch) = 0;
    virtual int MPI_Barrier(MPI_Comm comm) = 0;
    virtual double MPI_Wtime(void) = 0;
    virtual int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err) = 0;
    virtual int MPI_Reduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm) = 0;
    virtual int MPI_Allreduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type, MPI_Op op, MPI_Comm comm) = 0;
    virtual int MPI_Abort(MPI_Comm comm, int errorcode) = 0;
    //virtual int MPIX_Checkpoint(MPI_Comm comm) = 0;
};

}} // exampi::i
#endif
