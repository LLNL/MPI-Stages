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
	size_t extent;
	bool associative;
	bool weakAssociative;
	bool commutative;

public:
	// TODO THIS IS A NULL REFERENCE
	// TODO Why do we need this?
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

	size_t getExtent() const
	{
		return extent;
	}

	bool isAssociative() const
	{
		return associative;
	}

	bool isWeakAssociative() const
	{
		return weakAssociative;
	}

	bool isCommutative() const
	{
		return commutative;
	}
};







} // exampi

#endif // guard
