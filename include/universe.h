#ifndef __EXAMPI_UNIVERSE_H
#define __EXAMPI_UNIVERSE_H

namespace exampi
{

// TODO make singleton at the moment
class Universe
{
	// hold communicators
	// hold groups
	// ...

	Universe()
	{
		exampi::groups.push_back(group);

		communicator = new Comm(true, group, group);
		communicator->set_rank(exampi::rank);
		communicator->set_context(0, 1);

		exampi::communicators.push_back(communicator);
	}

	~Universe()
	{
		for(auto &&com : exampi::communicators)
	  	{
        	delete com;
	    }
        exampi::communicators.clear();
       
        // delete groups
		for (auto &&group : exampi::groups)
        {
        	delete group;
        }
        exampi::groups.clear();
	}
};
#endif
