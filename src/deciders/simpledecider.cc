// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

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
			[[fallthrough]];

		case Operation::Bsend:
			[[fallthrough]];

		case Operation::Rsend:
			return Protocol::EAGER;

		case Operation::Receive:
			throw SimpleDeciderReceiveDecisionException();

		default:
			throw SimpleDeciderUnknownException();
	}
}

}
