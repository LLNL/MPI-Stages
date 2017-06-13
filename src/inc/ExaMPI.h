/* Internal include for ExaMPI
 */

namespace exampi
{

typedef int MPI_Comm;
typedef void *MPI_Datatype;
typedef struct MPI_Status {
	int count;
	int cancelled;
	int MPI_SOURCE;
	int MPI_TAG;
	int MPI_ERROR;
} MPI_Status;

class ICheckpoint
{
  public:
    virtual void DoSomething() = 0;
};

class IFault
{
  public:
    virtual void DoSomething() = 0;
};

class IInterface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

class IMemory
{
  public:
    virtual void DoSomething() = 0;
};

class IProgress
{
  public:
    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

class ITransport
{
  public:
	virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
	virtual int recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

}
