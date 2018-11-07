#ifndef __EXAMPI_DATATYPE_H
#define __EXAMPI_DATATYPE_H

#include <mpi.h>
#include <unordered_map>

namespace exampi
{

class Datatype
{
protected:
	const MPI_Datatype &mpiDatatype;
	const size_t extent;
	const bool associative;
	const bool weakAssociative;
	const bool commutative;
public:
	Datatype() : mpiDatatype(0), extent(0), associative(false),
		weakAssociative(false), commutative(false) {;}
	Datatype(const MPI_Datatype &dt, size_t ex, bool assoc, bool wassoc, bool comm)
		: mpiDatatype(dt), extent(ex), associative(assoc), weakAssociative(wassoc),
		  commutative(comm) {;}
	virtual ~Datatype() {;}

	const MPI_Datatype &getMpiDatatype() const
	{
		return mpiDatatype;
	}
	const size_t getExtent() const
	{
		return extent;
	}
	const bool isAssociative() const
	{
		return associative;
	}
	const bool isWeakAssociative() const
	{
		return weakAssociative;
	}
	const bool isCommutative() const
	{
		return commutative;
	}
};







} // exampi

#endif // guard
