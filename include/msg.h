#ifndef __EXAMPI_MSG_H
#define __EXAMPI_MSG_H

namespace exampi
{

// packet.h:  data packet related functions (not specific to transport)
// this may need to be moved into a module, etc

namespace msg
{
class Header
{
  public:
    constexpr static int SIZE = 4;
    char *buf[SIZE];

    Header() {;} 

    tag getUserTag() {return static_cast<uint32_t *>(buf)[0];}
    tag getInternalTag() {return static_cast<uint32_t *>(buf)[1];}
    tag getSource() {return static_cast<uint32_t *>(buf)[2];}
    tag getContext() {return static_cast<uint32_t *>(buf)[3];}
    
    void setUserTag(tag t) {static_cast<uint32_t *>(buf)[0] = t;}
    void setInternalTag(tag t) {static_cast<uint32_t *>(buf)[1] = t;}
    void setSource(int r) {static_cast<uint32_t *>(buf)[2] = r;}
    void setContext(uint32_t c) {static_cast<uint32_t *>(buf)[3] = c;}

    // can probably convert this to a return-by-move for speed
    void toIovec(struct iovec *iov)
    {
      iov.iov_base = buf;
      iov.iov_len = SIZE;
    }
};


} // msg












} // exampi

#endif

