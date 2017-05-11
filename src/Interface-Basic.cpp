#include <ExaMPI.h>
#include <iostream>
#include <string>

namespace exampi
{

class BasicInterface : public IInterface
{
  private:
    int rank;  // TODO: Don't keep this here, hand off to progress
  public:
    BasicInterface() : rank(0){};
    virtual int MPI_Init(int *argc, char ***argv)
    {
      std::string arghdr("MPIARGST");
      std::string argftr("MPIARGEN");
      if(arghdr.compare(**argv) != 0)
      {
        // already initialized or other thread initializing, return
        return 0;
      }
      // for now, use fixed format of (hdr rank tportopts ftr)
      (*argv)++;
      (*argc)--;
      rank = std::stoi(**argv);
      (*argv)++;
      (*argc)--;
      // ignore transport opts for now
      (*argv)++;
      (*argc)--;
      // assume footer was there and correct
      return 0;
    }
    virtual int MPI_Finalize() { return 0; }
};

// This is an ugly, temporary hack -- we need compile-time selection
static BasicInterface bint;
}
