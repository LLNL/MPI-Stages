/* Internal include for ExaMPI
 */

namespace exampi
{

class ICheckpoint
{
  public:
    virtual void DoSomething() = 0;
};

class IFault
{
  public:
    virtual void DoSomething() = 0;
};

class IInterface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
};

class IMemory
{
  public:
    virtual void DoSomething() = 0;
};

class IProgress
{
  public:
    virtual void DoSomething() = 0;
};

class ITransport
{
  public:
    virtual void DoSomething() = 0;
};

}
