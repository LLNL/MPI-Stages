#include <errHandler.h>

namespace exampi {

volatile std::sig_atomic_t errHandler::is_errSet = 0;

errHandler::errHandler() {}

errHandler::~errHandler() {}

bool errHandler::setErrToHandle(int sig)
{
    if(std::signal(sig, errHandler::setErr) == SIG_ERR) {
        return false;
    }
    return true;
}

void errHandler::setErr(int unused)
{
	is_errSet = 1;
	exampi::global::progress->cleanUp();
}

int errHandler::isErrSet()
{
    return is_errSet;
}

}
