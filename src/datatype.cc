#include "datatype.h"

namespace exampi
{

Datatype::Datatype()
{
	;
}

Datatype::Datatype(MPI_Datatype datatype, size_t extent, bool assoc,
                   bool wassoc, bool comm) :
	datatype(datatype),
	extent(extent),
	associative(assoc),
	weak_associative(wassoc),
	commutative(comm)
{
	;
}

Datatype::~Datatype()
{
	;
}

MPI_Datatype Datatype::get_datatype() const
{
	return datatype;
}

size_t Datatype::get_extent() const
{
	return extent;
}

bool Datatype::is_associative() const
{
	return associative;
}

bool Datatype::is_weak_associative() const
{
	return weak_associative;
}

bool Datatype::is_commutative() const
{
	return commutative;
}

}
