#include <ExaMPI.h>
#include <UDPSocket.h>
#include <fstream>

namespace exampi
{

class BasicTransport : public ITransport
{
  private:
    std::string address;
    uint16_t port;
    std::vector<std::string> hostsv;
  public:
    BasicTransport() {};
        
    virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      //TODO: figure out destination ip and port from dest
      address = hostsv[dest].c_str();
      port = 8080;
      try {
        UDPSocket sock;
        sock.send(buf, count, address, port);
      } catch (std::exception &ex) {
          std::cerr << ex.what() << std::endl;
          exit(1);  
      }
    }
    
    virtual int recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      //TODO: figure out port from source
      port = 8080;
      try {
         UDPSocket sock(port);
         std::string sourceAddress;
         uint16_t sourcePort;
         int recvDataSize;
         for (;;) {
            recvDataSize = sock.recv(buf, count, sourceAddress, sourcePort);
            std::cout << "Received packet from " << sourceAddress << " : " << sourcePort << " : " << buf << std::endl;
         }
      } catch (std::exception &ex) {
          std::cerr << ex.what() << std::endl;
          exit(1);
      }
      return 0;
    }

    void SetHosts(std::vector<std::string> h)
    {
      hostsv = h;
    }
};
class BasicProgress : public IProgress
{
  private:
    BasicTransport btransport;
    std::vector<std::string> hosts;

  public:
    BasicProgress() : btransport() {};
        
    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      btransport.send(buf, count, datatype, dest, tag, comm);
      return 0;
    }
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      btransport.recv(buf, count, datatype, source, tag, comm, status);
      return 0;
    }

    void SetHosts(std::vector<std::string> h) 
    {
      btransport.SetHosts(h);
    }
};

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
      std::ifstream hostsfh("mpihosts.stdin.tmp", std::ifstream::in);
      std::string tmp;
      while(std::getline(hostsfh, tmp))
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
    	bprogress.recv_data(buf, count, datatype, source, tag, comm, status);
    	return 0;
    }
};

} // namespace exampi
