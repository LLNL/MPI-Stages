#ifndef __EXAMPI_ABSTRACT_STAGES_H
#define __EXAMPI_ABSTRACT_STAGES_H

#include <fsteam>

namespace exampi
{

class Stages
{
public:
	virtual ~Stages() {}

	virtual void save(std::ostream &) = 0;
	virtual void load(std::istream &) = 0;

	virtual void halt() = 0;
	virtual void clean() = 0;
	virtual void start() = 0;
};

} // exampi::i

#endif
