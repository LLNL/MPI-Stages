#ifndef __EXAMPI_REQUEST_H
#define __EXAMPI_REQUEST_H

#include <mutex>
#include <condition_variable>

#include "datatype.h"
#include "mpi.h"

namespace exampi
{

enum class Op: int
{
	Send,
	Receive
};

// 
extern thread_local std::mutex thr_request_lock;
extern thread_local std::condition_variable thr_request_condition;

struct Request
{
	std::condition_variable *condition;

	volatile bool complete; // if true no longer in protocol queue
	bool cancelled;
	bool persistent;
	bool active;

	Op op;
	int tag;
	int source;
	int destination;
	MPI_Comm communicator;
	const void *buffer;
	Datatype datatype;
	int count;
	int stage;

	Request();

	//void pack()
	//{
	//	uint16_t *word = (uint16_t *) hdr;
	//	uint32_t *dword = (uint32_t *) hdr;
	//	word[0] = 0xDEAF; // magic word
	//	word[1] = 22;  // protocol
	//	word[2] = 42;  // message type
	//	word[3] = 0x0; // function
	//	dword[2] = 0x0;  // align
	//	dword[3] = stage;  // align/reserved
	//	dword[4] = source;
	//	dword[5] = tag;
	//	dword[6] = comm; // context; not yet
	//	dword[7] = 0xAABBCCDD;  // CRC
	//}

	//void unpack()
	//{
	//	uint32_t *dword = (uint32_t *) hdr;
	//	stage = dword[3];
	//	source = dword[4];
	//	tag = dword[5];
	//	comm = dword[6];
	//}
};

}

#endif
