#ifndef __EXAMPI_DATATYPE_H
#define __EXAMPI_DATATYPE_H

#include <mpi.h>
#include <unordered_map>

namespace exampi
{

class Datatype
{
protected:
	MPI_Datatype datatype;

	size_t extent;
	bool associative;
	bool weak_associative;
	bool commutative;

public:
	Datatype();
	Datatype(MPI_Datatype dt, size_t ex, bool assoc, bool wassoc, bool comm);

	~Datatype();

	MPI_Datatype get_datatype() const;
	size_t get_extent() const;

	bool is_associative() const;
	bool is_weak_associative() const;
	bool is_commutative() const;
};

} // exampi

#endif // guard
