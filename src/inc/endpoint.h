#ifndef __EXAMPI_ENDPOINT_H
#define __EXAMPI_ENDPOINT_H

#include <mpi.h>
#include <datatype.h>


namespace exampi {

class Endpoint
{
  public:
    int rank;
    MPI_Comm comm;

    void invalid() { rank = -1; comm = -1; }
};

} // ::exampi

#endif
