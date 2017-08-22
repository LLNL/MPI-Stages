#include <ExaMPI.h>
#include <basic/interface.h>
#include <basic/progress.h>
#include <basic/transport.h>

// global definitions
// NOTE:  This is where you would use preproc (or w/e)
// to determine modules built and linked at runtime,
// by changing what is instantiated (in other words,
// ideally we'd have heavy preproc or generation of
// this file)   --sf

namespace exampi {
namespace global {

// NOTE:  I construct the objects here to give them the chance
// to do preinit at startup -- init calls will be required anyway
// to avoid OoI problems, so might as well take advantage of them.
exampi::Config *config = new exampi::Config();
exampi::i::Interface *interface = new exampi::basic::Interface();
exampi::i::Progress  *progress = new exampi::basic::Progress();
exampi::i::Transport *transport = new exampi::basic::Transport();

} // global
} // exampi
