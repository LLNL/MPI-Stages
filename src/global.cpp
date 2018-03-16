#include <global.h>

#include <basic/interface.h>
#include <basic/progress.h>
#include <basic/transport.h>
#include <basic/checkpoint.h>

// global definitions
// NOTE:  This is where you would use preproc (or w/e)
// to determine modules built and linked at runtime,
// by changing what is instantiated (in other words,
// ideally we'd have heavy preproc or generation of
// this file)   --sf

namespace exampi {
namespace global {

int rank = -1;
int worldSize = -1;
int epoch = -1;
std::string epochConfig;

// NOTE:  I construct the objects here to give them the chance
// to do preinit at startup -- init calls will be required anyway
// to avoid OoI problems, so might as well take advantage of them.
exampi::Config *config = new exampi::Config();
exampi::i::Interface *interface = new exampi::basic::Interface();
exampi::i::Progress  *progress = new exampi::basic::Progress();
exampi::i::Transport *transport = new exampi::basic::Transport();
exampi::i::Checkpoint *checkpoint = new exampi::basic::Checkpoint();

std::unordered_map<MPI_Datatype, exampi::Datatype> datatypes =
{

  { MPI_BYTE, Datatype(MPI_BYTE,           sizeof(unsigned char),  true,  true, true)},
  { MPI_CHAR, Datatype(MPI_CHAR,           sizeof(char),           true,  true, true)},
#if 0
  { MPI_WCHAR, Datatype(MPI_WCHAR,          sizeof(wchar_t),        true,  true, true)},
#endif
  { MPI_UNSIGNED_CHAR,  Datatype(MPI_UNSIGNED_CHAR,  sizeof(unsigned char),  true,  true, true)},
  { MPI_SHORT,          Datatype(MPI_SHORT,          sizeof(short),          true,  true, true)},
  { MPI_UNSIGNED_SHORT, Datatype(MPI_UNSIGNED_SHORT, sizeof(unsigned short), true,  true, true)},
  { MPI_INT,            Datatype(MPI_INT,            sizeof(int),            true,  true, true)},
  { MPI_UNSIGNED_INT,   Datatype(MPI_UNSIGNED_INT,   sizeof(unsigned int),   true,  true, true)},
  { MPI_LONG,           Datatype(MPI_LONG,           sizeof(long),           true,  true, true)},
  { MPI_UNSIGNED_LONG,  Datatype(MPI_UNSIGNED_LONG,  sizeof(unsigned long),  true,  true, true)},
  { MPI_FLOAT,          Datatype(MPI_FLOAT,          sizeof(float),          false, true, true)},
  { MPI_DOUBLE,         Datatype(MPI_DOUBLE,         sizeof(double),         false, true, true)},
#if 0
  { MPI_LONG_DOUBLE, Datatype(MPI_LONG_DOUBLE,    sizeof(long double),    false, true, true)},
#endif
};

} // global
} // exampi