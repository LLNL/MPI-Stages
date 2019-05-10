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

#ifndef __EXAMPI_ABSTRACT_STAGES_H
#define __EXAMPI_ABSTRACT_STAGES_H

#include <fstream>
#include <iostream>

namespace exampi
{

class Stages
{
public:
	virtual ~Stages() {}

	virtual int save(std::ostream &) = 0;
	virtual int load(std::istream &) = 0;

	virtual int halt() = 0;
	virtual int cleanup() = 0;
};

} // exampi::i

#endif
