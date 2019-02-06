#ifndef __EXAMPI_GLOBAL_H
#define __EXAMPI_GLOBAL_H

#include <vector>
#include <string>
#include <list>
#include <memory>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <mpi.h>

#include "datatype.h"

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <future>

#include <comm.h>
#include <group.h>
#include <errhandler.h>

#include <map>

#include <abstract/progress.h>
#include <abstract/transport.h>
#include <abstract/checkpoint.h>

namespace exampi
{

//extern int rank;
//extern int worldSize;
//extern std::string epochConfig;
//extern int epoch;
//
//extern exampi::Progress *progress;
//extern exampi::Transport *transport;

extern std::unordered_map<MPI_Datatype, exampi::Datatype> datatypes;
//extern exampi::Checkpoint *checkpoint;
//extern std::vector<exampi::Comm *> communicators;
//extern std::vector<exampi::Group *> groups;
//extern exampi::errHandler *handler;

} // namespace exampi

#endif
