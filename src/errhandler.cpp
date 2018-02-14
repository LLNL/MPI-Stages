
/*namespace exampi {

class errHandler : public exampi::i::EventHandler {
public:
	errHandler(void) : isErrSet(0) {}
	virtual int handle_signal(int signum) {
		assert (signum == SIGUSR2);
		this->isErrSet = 1;
	}

	sig_atomic_t errSet(void) {
		return this->isErrSet;
	}
private:
	sig_atomic_t isErrSet;
};
}*/
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
	//TODO: cleanup if error occurs
	is_errSet = 1;
}

int errHandler::isErrSet()
{
    return is_errSet;
}

}
