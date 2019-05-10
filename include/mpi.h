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

/*

   Exampi public "mpi.h"  V 0.0

   Authors: Shane Matthew Farmer, Nawrin Sultana, Anthony Skjellum

*/

#ifndef __EXAMPI_MPI_H
#define __EXAMPI_MPI_H

#include <stdint.h>
#include <sys/types.h>

#include "errors.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int MPI_Datatype;
typedef int MPI_Comm;
typedef int MPIX_Session;
typedef int MPI_Group;
typedef uintptr_t MPI_Request;
typedef int MPI_Op;
typedef int MPI_Info;
typedef int MPI_Errhandler;

typedef void (*MPI_User_Function)( void *invec, void *inoutvec, int *len,
                                   MPI_Datatype *datatype);

typedef struct
{
	ssize_t count;
	int cancelled;
	int MPI_SOURCE;
	int MPI_TAG;
	int MPI_ERROR;
} MPI_Status;

typedef struct
{
	MPI_Comm *comms;
	int comm_size;
	MPI_Group *grps;
	int group_size;
} MPIX_Handles;

typedef void (*MPIX_Serialize_handler) (MPIX_Handles *handle);
typedef void (*MPIX_Deserialize_handler) (MPIX_Handles handle);

//
#define MPI_STATUS_IGNORE 0
#define MPI_STATUSES_IGNORE 0
#define MPI_REQUEST_NULL 0
#define MPI_TAG_UB 65536
#define MPI_ANY_SOURCE -1
#define MPI_ANY_TAG -1


// TODO errors
#define MPI_REVERT 100  /* Need to revert to a checkpoint epoch */

#define MPIX_TRY_RELOAD 101
#define MPIX_SUCCESS_RECOVERY 102
#define MPIX_SUCCESS_RESTART 103
#define MPIX_CLEANUP_TAG 1
#define MPI_UNDEFINED -1
#define MPI_ERR_MPIEXEC 104
#define MPI_ERR_ABORT 255

/* other error classes not defined yet */


/* Basic datatypes */

#define MPI_BYTE             ((MPI_Datatype)0)
#define MPI_CHAR             ((MPI_Datatype)1)
#if 0
#define MPI_WCHAR            ((MPI_Datatype)2)
#endif
#define MPI_UNSIGNED_CHAR    ((MPI_Datatype)3)
#define MPI_SHORT            ((MPI_Datatype)4)
#define MPI_UNSIGNED_SHORT   ((MPI_Datatype)5)
#define MPI_INT              ((MPI_Datatype)6)
#define MPI_UNSIGNED_INT     ((MPI_Datatype)7)
#define MPI_LONG             ((MPI_Datatype)8)
#define MPI_UNSIGNED_LONG    ((MPI_Datatype)9)
#define MPI_FLOAT            ((MPI_Datatype)10)
#define MPI_DOUBLE           ((MPI_Datatype)11)
#define MPI_LONG_LONG_INT	 ((MPI_Datatype)12)
#define MPI_LONG_LONG		 ((MPI_Datatype)13)

#define MPI_FLOAT_INT       ((MPI_Datatype)64)
#define MPI_LONG_INT        ((MPI_Datatype)65)
#define MPI_DOUBLE_INT      ((MPI_Datatype)66)
#define MPI_2INT            ((MPI_Datatype)67)

#define MPI_AINT			((MPI_Datatype)101)

typedef struct float_int_type float_int_type;
typedef struct long_int_type long_int_type;
typedef struct double_int_type double_int_type;
typedef struct int_int_type	int_int_type;

struct float_int_type
{
	float val;
	int loc;
};

struct long_int_type
{
	long val;
	int loc;
};

struct double_int_type
{
	double val;
	int loc;
};

struct int_int_type
{
	int val;
	int loc;
};


#if 0 /* not supported yet */

#define MPI_LONG_DOUBLE     ((MPI_Datatype)12)

#define MPI_BOOL            ((MPI_Datatype)32)
#define MPI_FLOAT_INT       ((MPI_Datatype)64)
#define MPI_LONG_INT        ((MPI_Datatype)65)
#define MPI_DOUBLE_INT      ((MPI_Datatype)66)
#define MPI_SHORT_INT       ((MPI_Datatype)67)
#define MPI_2INT            ((MPI_Datatype)68)
#define MPI_LONG_DOUBLE_INT ((MPI_Datatype)69)


#define MPI_PACKED          ((MPI_Datatype)128)
#endif


/* Built-in collective ops go here: */

/* The basic set:

MPI_Op op :

MPI_MAX maximum
MPI_MIN minimum
MPI_SUM sum
MPI_PROD product
MPI_LAND logical and
MPI_BAND bit-wise and
MPI_LOR logical or
MPI_BOR bit-wise or
MPI_LXOR logical xor
MPI_BXOR bit-wise xor
MPI_MAXLOC max value and location
MPI_MINLOC min value and location

MPI_REPLACE  f(a,b) <-- b ; replace what you have.

MPI_OP_NULL too.

 */
#define MPI_OP_NULL ((MPI_Op)0)
#define MPI_MAX     ((MPI_Op)1)
#define MPI_MIN     ((MPI_Op)2)
#define MPI_SUM     ((MPI_Op)3)
#define MPI_MAXLOC	((MPI_Op)4)
#define MPI_MINLOC	((MPI_Op)5)


#if 0 /* not in first release */
MPI_Op MPI_PROD;
MPI_Op MPI_LAND;
MPI_Op MPI_BAND;
MPI_Op MPI_LOR;
MPI_Op MPI_BOR;
MPI_Op MPI_LXOR;
MPI_Op MPI_BXOR;
MPI_Op MPI_MAXLOC;
MPI_Op MPI_MINLOC;

MPI_Op MPI_REPLACE; /* check this one; definitely not in first release */
#endif


#define MPI_COMM_NULL     ((MPI_Comm)-1)
#define MPI_COMM_WORLD    ((MPI_Comm)0)

#define MPI_ERRORS_RETURN ((MPI_Errhandler)0)

/* Core MPI API */

int MPI_Abort(MPI_Comm, int);
int MPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype,
                  MPI_Comm);
int MPI_Allreduce(const void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int MPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype,
                 MPI_Comm);
int MPI_Barrier(MPI_Comm);
int MPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
int MPI_Comm_create(MPI_Comm, MPI_Group, MPI_Comm *);
int MPI_Comm_create_group(MPI_Comm, MPI_Group, int, MPI_Comm *);
int MPI_Comm_dup(MPI_Comm, MPI_Comm *);
int MPI_Comm_rank(MPI_Comm, int *);
int MPI_Comm_set_errhandler(MPI_Comm, MPI_Errhandler);
int MPI_Comm_size(MPI_Comm, int *);
int MPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
int MPI_Finalize(void);
int MPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
               MPI_Comm);
int MPI_Get_count(MPI_Status *, MPI_Datatype, int *);
int MPI_Group_create_session(MPIX_Session, char *, MPI_Group *);
int MPI_Iallreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm,
                   MPI_Request *);
int MPI_Ibcast(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *);
int MPIX_Icheckpoint(MPI_Comm, MPI_Request *);
int MPI_Igather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                MPI_Comm, MPI_Request *);
int MPI_Ireduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm,
                MPI_Request *);
int MPI_Iscatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                 MPI_Comm, MPI_Request *);
int MPI_Init(int *, char ***);
int MPI_Init_info(int *, char ***, MPI_Info);
int MPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm, MPI_Status *status);
int MPI_Recv_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPI_Reduce(const void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int MPI_Scatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                MPI_Comm);
int MPI_Send(const void *, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Sendrecv(const void *, int, MPI_Datatype, int, int, void *, int,
                 MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int MPI_Send_init(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int MPIX_Session_get_names(MPIX_Session, char **);
int MPIX_Session_init(MPI_Info, MPI_Errhandler, MPIX_Session *);
int MPIX_Session_finalize(MPIX_Session *);
int MPI_Start(MPI_Request *);
int MPI_Startall(int, MPI_Request *);
int MPI_Wait(MPI_Request *, MPI_Status *);
int MPI_Waitall(int, MPI_Request [], MPI_Status []);
double MPI_Wtime(void);

int MPIX_Checkpoint_write(void);
int MPIX_Checkpoint_read(void);
int MPIX_Get_fault_epoch(int *);
int MPIX_Serialize_handles();
int MPIX_Deserialize_handles();
int MPIX_Serialize_handler_register(const MPIX_Serialize_handler handler);
int MPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler);

#if 0
int MPI_Get_processor_name( char *name, int *resultlen );

int MPI_Comm_create_errhandler(MPI_Comm_errhandler_fn *function,
                               MPI_Errhandler *errhandler);
int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int MPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *errhandler);
int PMPI_Errhandler_free(MPI_Errhandler *errhandler);

/*
   minimum:

   MPI_ERRORS_ARE_FATAL
   MPI_ERRORS_RETURN
   MPI_ERR_HANDLER_NULL
*/


#endif

/* MPI Profiling API */

int PMPI_Abort(MPI_Comm, int);
int PMPI_Allgather(void *, int, MPI_Datatype, void *, int, MPI_Datatype,
                   MPI_Comm);
int PMPI_Allreduce(const void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int PMPI_Alltoall(void *, int, MPI_Datatype, void *, int, MPI_Datatype,
                  MPI_Comm);
int PMPI_Barrier(MPI_Comm);
int PMPI_Bcast(void *, int, MPI_Datatype, int, MPI_Comm);
int PMPI_Comm_create(MPI_Comm, MPI_Group, MPI_Comm *);
int PMPI_Comm_create_group(MPI_Comm, MPI_Group, int, MPI_Comm *);
int PMPI_Comm_dup(MPI_Comm, MPI_Comm *);
int PMPI_Comm_rank(MPI_Comm, int *);
int PMPI_Comm_set_handler(MPI_Comm, MPI_Errhandler);
int PMPI_Comm_size(MPI_Comm, int *);
int PMPI_Comm_split(MPI_Comm, int, int, MPI_Comm *);
int PMPI_Finalize(void);
int PMPI_Gather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                MPI_Comm);
int PMPI_Get_count(MPI_Status *, MPI_Datatype, int *);
int PMPI_Group_create_session(MPIX_Session, char *, MPI_Group *);
int PMPI_Iallreduce(void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm,
                    MPI_Request *);
int PMPI_Ibcast(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *);
int PMPIX_Icheckpoint(MPI_Comm, MPI_Request *);
int PMPI_Igather(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                 MPI_Comm, MPI_Request *);
int PMPI_Ireduce(void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm,
                 MPI_Request *);
int PMPI_Iscatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                  MPI_Comm, MPI_Request *);
int PMPI_Init(int *, char ***);
int PMPI_Init_info(int *, char ***, MPI_Info);
int PMPI_Irecv(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int PMPI_Isend(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Status *status);
int PMPI_Recv_init(void *, int, MPI_Datatype, int, int, MPI_Comm,
                   MPI_Request *);
int PMPI_Reduce(const void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int PMPI_Scatter(void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
                 MPI_Comm);
int PMPI_Send(const void *, int, MPI_Datatype, int, int, MPI_Comm);
int PMPI_Sendrecv(const void *, int, MPI_Datatype, int, int, void *, int,
                  MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int PMPI_Send_init(void *, int, MPI_Datatype, int, int, MPI_Comm,
                   MPI_Request *);
int PMPIX_Session_get_names(MPIX_Session, char **);
int PMPIX_Session_init(MPI_Info, MPI_Errhandler, MPIX_Session *);
int PMPIX_Session_finalize(MPIX_Session *);
int PMPI_Start(MPI_Request *);
int PMPI_Startall(int, MPI_Request *);
int PMPI_Wait(MPI_Request *, MPI_Status *);
int PMPI_Waitall(int, MPI_Request [], MPI_Status []);
double PMPI_Wtime(void);

int PMPIX_Checkpoint_write(void);
int PMPIX_Checkpoint_read(void);
int PMPIX_Get_fault_epoch(int *);
int PMPIX_Serialize_handles();
int PMPIX_Deserialize_handles();
int PMPIX_Serialize_handler_register(const MPIX_Serialize_handler handler);
int PMPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler);

#if 0
int PMPI_Get_processor_name( char *name, int *resultlen );

int PMPI_Comm_create_errhandler(MPI_Comm_errhandler_fn *function,
                                MPI_Errhandler *errhandler);
int PMPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int PMPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *errhandler);
int PMPI_Errhandler_free(MPI_Errhandler *errhandler);
#endif




#ifdef __cplusplus
}
#endif


#endif
