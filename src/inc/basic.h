#include <ExaMPI.h>
#include <UDPSocket.h>
#include <config.h>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <sys/uio.h>

namespace exampi
{

const std::string runtimeConfig("mpihosts.stdin.tmp");

class BasicBuf : public IBuf
{
  private:
    struct iovec v;
  public:
    BasicBuf(void *p, size_t sz) {v.iov_base = p; v.iov_len = sz;}
    virtual struct iovec iov() { return v;}
};


namespace udp
{

class Socket
{
  private:
    int fd;
  public:
    Socket() { fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); }
    ~Socket() { close(fd); }
    int getFd() { return fd; }
};

class Address
{
  private:
    sockaddr_in addr;
  public:
    void set(const std::string &ip, const uint16_t &port)
    {
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    Address() { memset(&addr, 0, sizeof(addr));}
    Address(std::string ip, uint16_t port)
    {
      Address();
      set(ip, port);
    }
    sockaddr_in *get() { return &addr; }
    socklen_t size() { return sizeof(addr); }
};

class Message
{
  private:  
    struct msghdr hdr;
    std::vector<struct iovec> iov;
  public:
    Message() : iov()
    {
      hdr.msg_control = NULL;
      hdr.msg_controllen = 0;
    }

    void addBuf(IBuf *b) { iov.push_back(b->iov()); }
    void updateHeader()
    {
      hdr.msg_iov = iov.data();
      hdr.msg_iovlen = iov.size();
    }
    void updateHeader(Address &addr)
    {
      updateHeader();
      hdr.msg_name = addr.get();
      hdr.msg_namelen = addr.size();
    }


    void send(Socket &sock, Address &addr) { updateHeader(addr); sendmsg(sock.getFd(), &hdr, 0); };
    void receive(Socket &sock) { updateHeader(); recvmsg(sock.getFd(), &hdr, 0); };
};

} // udp

class BasicTransport : public ITransport
{
  private:
    std::string address;
    uint16_t port;
    Config *config;
  public:
    BasicTransport(Config *c) : config(c) {};

    virtual void send(IBuf *buf, int dest, MPI_Comm comm)
    {
      std::string rank = std::to_string(dest);
      std::string destip = (*config)[rank];
      udp::Socket s;
      udp::Address addr(destip, 8080);
      udp::Message msg;

      msg.addBuf(buf);
      msg.send(s, addr);
    }

    virtual void receive(IBuf *buf, MPI_Comm comm)
    {
      udp::Socket s;
      udp::Message msg;

      msg.addBuf(buf);
      msg.receive(s);
    }
        
    virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      std::cout << "In BasicTransport.send(...,rank="<<dest<<",...)" << std::endl;
      std::string rank = std::to_string(dest);
      std::string destip = (*config)[rank];
      port = 8080;
      try {
        UDPSocket sock;
        sock.send(buf, count, destip.c_str(), port);
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
            recvDataSize = sock.recv(buf, count, sourceAddress, sourcePort);
            std::cout << "Received packet from " << sourceAddress << " : " << sourcePort << " : " << buf << std::endl;
      } catch (std::exception &ex) {
          std::cerr << ex.what() << std::endl;
          exit(1);
      }
      return 0;
    }

};
class BasicProgress : public IProgress
{
  private:
    Config *config;
    BasicTransport btransport;

  public:
    BasicProgress(Config *c) : config(c), btransport(config) {};
        
    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {

      BasicBuf b((void *)buf, count);
      btransport.send(&b, dest, comm );
      //btransport.send(buf, count, datatype, dest, tag, comm);
      return 0;
    }
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      BasicBuf b(buf, count);
      btransport.receive(&b, comm);
      //btransport.recv(buf, count, datatype, source, tag, comm, status);
      return 0;
    }

};

class BasicInterface : public IInterface
{
  private:
    int rank;  // TODO: Don't keep this here, hand off to progress
    int ranks;
    std::vector<std::string> hosts;
    Config config;
    BasicProgress bprogress;

  public:
    static BasicInterface *global;

    BasicInterface() : rank(0), config(), bprogress(&config) {};
    virtual int MPI_Init(int *argc, char ***argv)
    {

      // first param is config file, second is rank
      std::cout << "Loading config from " << **argv << std::endl;
      config.Load(**argv);
      (*argv)++;
      (*argc)--;
      std::cout << "Taking rank to be arg " << **argv << std::endl;
      rank = atoi(**argv);
      (*argv)++;
      (*argc)--;
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

    virtual int MPI_Comm_rank(MPI_Comm comm, int *r)
    {
      *r = rank;
      return 0;
    }

    virtual int MPI_Comm_size(MPI_Comm comm, int *r)
    {
      *r = std::stoi(config["size"]);
      return 0;
    }
};


} // namespace exampi
