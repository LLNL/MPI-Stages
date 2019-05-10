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

#ifndef __EXAMPI_MPISTAGES_H
#define __EXAMPI_MPISTAGES_H

#include "interfaces/interface.h"
#include "abstract/stages.h"

#include <sstream>

namespace exampi
{

class StagesInterface: public BasicInterface, virtual public Stages
{
private:
	int recovery_code;

	std::vector<MPIX_Serialize_handler> serialize_handlers;
	std::vector<MPIX_Deserialize_handler> deserialize_handlers;

	int checkpoint_write();
	int checkpoint_read();

public:
	StagesInterface();
	~StagesInterface();

	int MPI_Init(int *argc, char ***argv);
	int MPI_Finalize();

	int MPIX_Checkpoint_write();
	int MPIX_Checkpoint_read();
	int MPIX_Get_fault_epoch(int *epoch);
	int MPIX_Serialize_handles();
	int MPIX_Deserialize_handles();
	int MPIX_Serialize_handler_register(const MPIX_Serialize_handler handler);
	int MPIX_Deserialize_handler_register(const MPIX_Deserialize_handler handler);

	int save(std::ostream &);
	int load(std::istream &);
	int halt();
	int cleanup();

};

} //exampi

#endif
