// Placeholder MPI file pending further implementation
// 2017-04-26 SF


/* FIXME: These are dummy datatypes!  SF */
typedef MPI_Datatype int;
typedef MPI_Comm int;
typedef MPI_Session int;
typedef MPI_Group int;
typedef MPI_Request int;


/* Core MPI API */

int MPI_Abort(MPI_Comm, int)
int MPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm)
int MPI_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm)
int MPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm)
int MPI_Barrier(MPI_Comm)
int MPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm)
int MPI_Comm_create(MPI_Comm, MPI_Group, MPI_Comm *)
int MPI_Comm_create_group(MPI_Comm, MPI_Group, int, MPI_Comm *)
int MPI_Comm_rank(MPI_Comm, int *)
int MPI_Comm_size(MPI_Comm, int *)
int MPI_Comm_split(MPI_Comm, int, int, MPI_Comm *)
int MPI_Finalize(void)
int MPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm)
int MPI_Group_create_session(MPI_Session, char *, MPI_Group *)
int MPI_Iallreduce(void *, void *, int, MPI_datatype, MPI_Op, MPI_Comm, MPI_Request *)
int MPI_Ibcast(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int MPI_Icheckpoint(MPI_Comm, MPI_Request *)
int MPI_Igather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int MPI_Ireduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm, MPI_Request *)
int MPI_Iscatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int MPI_Init(int *, char ***)
int MPI_Init_info(int *, char ***, MPI_Info)
int MPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int MPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int MPI_Recv_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int MPI_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm)
int MPI_Scatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm)
int MPI_Send_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int MPI_Session_get_names(MPI_Session, char **)
int MPI_Session_init(MPI_Info, MPI_Errhandler, MPI_Session *)
int MPI_Session_finalize(MPI_Session *)
int MPI_Start(MPI_Request *)
int MPI_Startall(int, MPI_Request *)
int MPI_Wait(MPI_Request *, MPI_Status *)
int MPI_Waitall(int, MPI_Request *, MPI_Status *)
int MPI_Wtime(void)

/* MPI Profiling API */

int PMPI_Abort(MPI_Comm, int)
int PMPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm)
int PMPI_Allreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm)
int PMPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm)
int PMPI_Barrier(MPI_Comm)
int PMPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm)
int PMPI_Comm_create(MPI_Comm, MPI_Group, MPI_Comm *)
int PMPI_Comm_create_group(MPI_Comm, MPI_Group, int, MPI_Comm *)
int PMPI_Comm_rank(MPI_Comm, int *)
int PMPI_Comm_size(MPI_Comm, int *)
int PMPI_Comm_split(MPI_Comm, int, int, MPI_Comm *)
int PMPI_Finalize(void)
int PMPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm)
int PMPI_Group_create_session(MPI_Session, char *, MPI_Group *)
int PMPI_Iallreduce(void *, void *, int, MPI_datatype, MPI_Op, MPI_Comm, MPI_Request *)
int PMPI_Ibcast(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int PMPI_Icheckpoint(MPI_Comm, MPI_Request *)
int PMPI_Igather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int PMPI_Ireduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm, MPI_Request *)
int PMPI_Iscatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *)
int PMPI_Init(int *, char ***)
int PMPI_Init_info(int *, char ***, MPI_Info)
int PMPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int MPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int PMPI_Recv_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int PMPI_Reduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm)
int PMPI_Scatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm)
int PMPI_Send_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
int PMPI_Session_get_names(MPI_Session, char **)
int PMPI_Session_init(MPI_Info, MPI_Errhandler, MPI_Session *)
int PMPI_Session_finalize(MPI_Session *)
int PMPI_Start(MPI_Request *)
int PMPI_Startall(int, MPI_Request *)
int PMPI_Wait(MPI_Request *, MPI_Status *)
int PMPI_Waitall(int, MPI_Request *, MPI_Status *)
int PMPI_Wtime(void)
