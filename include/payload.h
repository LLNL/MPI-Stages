#ifndef __EXAMPI_PAYLOAD_H
#define __EXAMPI_PAYLOAD_H

#include "datatype.h"

namespace exampi
{

struct Payload
{
	const void *buffer;
	MPI_Datatype datatype;
	int count;

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
