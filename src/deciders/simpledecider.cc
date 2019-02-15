#include "deciders/simpledecider.h"

namespace exampi
{

Protocol SimpleDecider::decide(const Request_ptr request,
                               const Universe &universe) const
{
	switch(request->operation)
	{
		case Operation::Ssend:
			return Protocol::EAGER_ACK;

		case Operation::Send:
			[[fallthrough]]

		case Operation::Bsend:
			[[fallthrough]]

		case Operation::Rsend:
			return Protocol::EAGER;

		case Operation::Receive:
			throw SimpleDeciderReceiveDecisionException();

		default:
			throw SimpleDeciderUnknownException();
	}
}

}
