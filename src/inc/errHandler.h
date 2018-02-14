#ifndef __EXAMPI_ERRHANDLER_H
#define __EXAMPI_ERRHANDLER_H

#include <iostream>
#include <csignal>

namespace exampi {
class errHandler
{
public:
    errHandler();
    ~errHandler();

    bool setErrToHandle(int sig);
    static int isErrSet();
    static void setErr(int unused);

private:
    volatile static std::sig_atomic_t is_errSet;
};
}
#endif
