#ifndef __EXAMPI_GLOBAL_H
#define __EXAMPI_GLOBAL_H

#include <vector>
#include <string>
#include <list>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <mpi.h>
#include <datatypes.h>
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

#include <config.h>
#include <datatype.h>

#include <map>
#include <array.h>
#include <i/interface.h>
#include <i/progress.h>
#include <i/transport.h>
#include <i/checkpoint.h>

namespace exampi {
namespace global {
    extern int rank;
  extern int worldSize;
  extern std::string epochConfig;
  extern int epoch;
  extern exampi::Config *config;
  extern exampi::i::Interface *interface;
  extern exampi::i::Progress *progress;
  extern exampi::i::Transport *transport;
  extern std::unordered_map<MPI_Datatype, exampi::Datatype> datatypes;
  extern exampi::i::Checkpoint *checkpoint;

}} // exampi::i
#endif
