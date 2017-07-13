#include <ExaMPI.h>
#include <iostream>
#include <string>

namespace exampi
{

class BasicInterface : public IInterface
{
  private:
    int rank;  // TODO: Don't keep this here, hand off to progress
    int ranks;
    std::vector<std::string> hosts;
    BasicProgress bprogress;

  public:
    BasicInterface() : rank(0), bprogress() {};
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
      ranks = std::stoi(**argv);
      (*argv)++;
      (*argc)--;
      // assume footer was there and correct
      //
      // warning:  magic word
      std::ifstream hostsfh("mpihosts.stdin.tmp");
      std::string tmp;
      while(std::getline(hostsfh, temp))
      {
        hosts.push_back(tmp);
      }

      bprogress.SetHosts(hosts);
      return 0;
    }
    virtual int MPI_Finalize() { return 0; }

    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    	bprogress.send_data(buf, count, datatype, dest, tag, comm);
    	return 0;
    }

    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    	bprogress.recv_data(buf, count, datatype, source, tag, comm, &status);
    	return 0;
    }
};

// This is an ugly, temporary hack -- we need compile-time selection
static BasicInterface bint;
}
