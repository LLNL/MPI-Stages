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
