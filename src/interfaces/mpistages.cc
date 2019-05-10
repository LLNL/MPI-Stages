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

#include "interfaces/mpistages.h"
#include "engines/blockingprogress.h"
#include "checks.h"
#include "debug.h"

namespace exampi
{

StagesInterface::StagesInterface()
{
	;
}

StagesInterface::~StagesInterface()
{
	;
}

int StagesInterface::MPI_Init(int *argc, char ***argv)
{
	debug("MPI_Init entered. argc=" << *argc);

	// check that exampi-mpiexec was used to launch the application
	if(std::getenv("EXAMPI_MONITORED") == NULL)
	{
		debug("Application was not launched with mpiexec.");
		return MPI_ERR_MPIEXEC;
	}
	debug("MPI_Init passed EXAMPI_LAUNCHED check.");

	Universe &universe = Universe::get_root_universe();
	universe.initialize();

	recovery_code = MPI_SUCCESS;

	universe.progress = std::make_unique<BlockingProgress>();

	debug("generating universe datatypes");
	init_datatype();

	// initialize in first run
	if (universe.epoch == 0)
	{
		// MPI WORLD GROUP
		debug("generating world group");
		init_group();

		// MPI_COMM_WORLD
		debug("generating world communicator");
		init_communicator();
	}
	else
	{
		recovery_code = checkpoint_read();
		if(recovery_code != MPI_SUCCESS)
		{
			debug("unsuccessful checkpoint load");
			return recovery_code;
		}
	}

	// execute global barrier
	// todo mpi stages move to checkpoint
	if(universe.epoch == 0)
	{
		debug("executing daemon barrier " << universe.rank);

		Daemon &daemon = Daemon::get_instance();
		daemon.barrier();
		// todo if done for udp non-recv reason, then this should live in udp transport
		//      not all transports require a barrier
	}

	// todo Nawrin?
	/* Checkpoint/restart
	 * errHandler handler;
	 * handler.setErrToHandle(SIGUSR2);
	 */

	debug("Finished MPI_Init with code: " << recovery_code);
	return recovery_code;
}

int StagesInterface::MPI_Finalize()
{
	debug("MPI_Finalize");

	serialize_handlers.clear();
	deserialize_handlers.clear();

	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Serialize_handles()
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	std::stringstream filename;
	filename << universe.epoch - 1 << "." << universe.rank << ".cp";
	std::ofstream t(filename.str(), std::ofstream::out | std::ofstream::app);

	if (!serialize_handlers.empty())
	{
		for (auto it : serialize_handlers)
		{
			MPIX_Handles handles;
			(*it)(&handles);
			t.write(reinterpret_cast<char *>(&handles.comm_size), sizeof(int));
			for (int i = 0; i < handles.comm_size; i++)
			{
				std::shared_ptr<Comm> c = universe.communicators.at(handles.comms[i]);
				//Comm *c = universe.communicators.at(handles.comms[i]);
				int id = c->get_context_id_pt2pt();
				t.write(reinterpret_cast<char *>(&id), sizeof(int));
			}

			t.write(reinterpret_cast<char *>(&handles.group_size), sizeof(int));
			for (int i = 0; i < handles.group_size; i++)
			{
				std::shared_ptr<Group> g = universe.groups.at(handles.grps[i]);
				//Group *g = universe.groups.at(handles.grps[i]);
				int id = g->get_group_id();
				t.write(reinterpret_cast<char *>(&id), sizeof(int));
			}
		}
	}
	t.close();

	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Deserialize_handles()
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
	std::stringstream filename;
	filename << universe.epoch - 1 << "." << universe.rank << ".cp";
	std::ifstream t(filename.str(), std::ifstream::in);

	long long int pos;
	t.read(reinterpret_cast<char *>(&pos), sizeof(long long int));
	t.seekg(pos);

	int size, id;

	if (!deserialize_handlers.empty())
	{
		for (auto iter : deserialize_handlers)
		{
			MPIX_Handles handles;
			t.read(reinterpret_cast<char *>(&size), sizeof(int));
			handles.comm_size = size;
			if (handles.comm_size > 0)
			{
				handles.comms = (MPI_Comm *)malloc(size * sizeof(MPI_Comm));
				for (int i = 0; i < size; i++)
				{
					t.read(reinterpret_cast<char *>(&id), sizeof(int));
					auto it = std::find_if(universe.communicators.begin(),
					                       universe.communicators.end(),
					                       [id](std::shared_ptr<Comm> &i) -> bool {return i->get_context_id_pt2pt() == id;});
					if (it == universe.communicators.end())
					{
						return MPIX_TRY_RELOAD;
					}
					else
					{
						handles.comms[i] = std::distance(universe.communicators.begin(), it);
					}
				}
			}
			t.read(reinterpret_cast<char *>(&size), sizeof(int));
			handles.group_size = size;
			if (handles.group_size > 0)
			{
				handles.grps = (MPI_Group *)malloc(size * sizeof(MPI_Group));
				for (int i = 0; i < size; i++)
				{
					t.read(reinterpret_cast<char *>(&id), sizeof(int));
					auto it = std::find_if(universe.groups.begin(),
					                       universe.groups.end(),
					                       [id](std::shared_ptr<Group> &i) -> bool {return i->get_group_id() == id;});
					if (it == universe.groups.end())
					{
						return MPIX_TRY_RELOAD;
					}
					else
					{
						handles.grps[i] = std::distance(universe.groups.begin(), it);
					}
				}
			}
			(*iter)(handles);
		}
	}

	t.close();
	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Serialize_handler_register(const
        MPIX_Serialize_handler
        handler)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
	if (universe.epoch == 0 && recovery_code == MPI_SUCCESS)
	{
		serialize_handlers.push_back(handler);
	}
	else if (universe.epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
	{
		serialize_handlers.push_back(handler);
	}
	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Deserialize_handler_register(const
        MPIX_Deserialize_handler handler)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
	if (universe.epoch == 0 && recovery_code == MPI_SUCCESS)
	{
		deserialize_handlers.push_back(handler);
	}
	else if (universe.epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
	{
		deserialize_handlers.push_back(handler);
	}

	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Checkpoint_write()
{
	CHECK_STAGES_ERROR();

	//Universe &universe = Universe::get_root_universe();
	//universe.stages->save();
	return checkpoint_write();
}

int StagesInterface::MPIX_Checkpoint_read()
{
	FaultHandler &faulthandler = FaultHandler::get_instance();
	if(faulthandler.isErrSet())
	{
		debug("resetting fault handler");
		faulthandler.setErrToZero();
	}

	Universe &universe = Universe::get_root_universe();
	debug("surviving process restart " << universe.rank);

	// wait for restarted process
	Daemon &daemon = Daemon::get_instance();

	daemon.wait_commit();
	debug("commit epoch received " << universe.epoch);

	// todo again why is there a barrier, due to transport? then should live in transport
	//      otherwise clarify and put in corrrect place instead of in interface, because
	//      the interface does not require it fundementally.
	debug("entering daemon barrier for restart");
	daemon.barrier();

	return MPI_SUCCESS;
}

int StagesInterface::MPIX_Get_fault_epoch(int *epoch)
{
	CHECK_STAGES_ERROR();

	// fetch universe
	Universe &universe = Universe::get_root_universe();
	*epoch = universe.epoch;
	debug("current epoch " << *epoch);

	return MPI_SUCCESS;
}

int StagesInterface::checkpoint_write()
{
	int err = MPI_SUCCESS;

	Universe &universe = Universe::get_root_universe();

	std::stringstream filename;
	filename << universe.epoch << "." << universe.rank << ".cp";
	debug("opening file " << filename.str());

	std::ofstream output_file(filename.str(), std::ofstream::out);

	if(!output_file.is_open())
		//debug("error opening checkpoint file");
		//return MPIX_TRY_RELOAD;
		throw std::runtime_error("Opening checkpoint file failed during writing.");

	long long int size = 0;
	long long int begin = output_file.tellp();
	output_file.write(reinterpret_cast<char *>(&size), sizeof(long long int));

	// save universe
	err = universe.interface->save(output_file);
	err = universe.progress->save(output_file);

	long long int end = output_file.tellp();
	size = end - begin;
	output_file.clear();
	output_file.seekp(0, std::ofstream::beg);

	output_file.write(reinterpret_cast<char *>(&size), sizeof(long long int));
	output_file.close();

	FaultHandler &faulthandler = FaultHandler::get_instance();
	if(faulthandler.isErrSet() != 1)
	{
		// NOTE needed in case process dies
		// NOTE could also send to daemon

		// write out epoch number
		debug("incrementing and outputting epoch " << universe.epoch << " -> " <<
		      (universe.epoch+1));
		universe.epoch++;

		std::ofstream epoch_config(universe.epoch_config);
		epoch_config << universe.epoch;
		epoch_config.close();
	}

	return err;
}

int StagesInterface::checkpoint_read()
{
	int err = MPI_SUCCESS;
	Universe &universe = Universe::get_root_universe();
	debug("epoch " << universe.epoch << ", reading checkpoint");
	debug("universe restart");

	// get a file.  this is actually nontrivial b/c of shared filesystems;
	std::stringstream filename;
	filename << universe.epoch - 1 << "." << universe.rank << ".cp";
	debug("opening file " << filename.str());

	std::ifstream input_file(filename.str(), std::ifstream::in);

	if(!input_file.is_open())
		//debug("error opening checkpoint file");
		//return MPIX_TRY_RELOAD;
		throw std::runtime_error("Opening checkpoint file failed during reading.");

	long long int pos;
	input_file.read(reinterpret_cast<char *>(&pos), sizeof(long long int));

	err = universe.interface->load(input_file);
	err = universe.progress->load(input_file);

	Daemon &daemon = Daemon::get_instance();
	debug("daemon barrier after restart");
	daemon.barrier();

	input_file.close();

	// read in epoch number
	// todo is this still required?

	std::ifstream epoch_config(universe.epoch_config);
	epoch_config >> universe.epoch;
	epoch_config.close();

	debug("loaded epoch " << universe.epoch);

	return err;
}

int StagesInterface::save(std::ostream &t)
{
	Universe &universe = Universe::get_root_universe();

	// save groups
	int group_size = universe.groups.size();
	t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
	for (auto &g : universe.groups)
	{
		int value = g->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = g->get_process_list().size();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		for (auto p : g->get_process_list())
		{
			t.write(reinterpret_cast<char *>(&p), sizeof(int));
		}
	}

	// save communicators
	int comm_size = universe.communicators.size();
	t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
	for(auto &c : universe.communicators)
	{
		int value = c->get_rank();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_pt2pt();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_coll();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		bool intra = c->get_is_intra();
		t.write(reinterpret_cast<char *>(&intra), sizeof(bool));
		value = c->get_local_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_remote_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
	}

	return MPI_SUCCESS;
}

int StagesInterface::load(std::istream &t)
{
	Universe &universe = Universe::get_root_universe();

	int comm_size, group_size;
	int r, p2p, coll, id;
	bool intra;
	int num_of_processes;
	std::list<int> ranklist;
	int rank;

	std::shared_ptr<Group> grp;
	std::shared_ptr<Comm> com;

	//restore group
	t.read(reinterpret_cast<char *>(&group_size), sizeof(int));
	while(group_size)
	{
		// todo heap allocation
		//grp = new exampi::Group();

		t.read(reinterpret_cast<char *>(&id), sizeof(int));
		//grp->set_group_id(id);
		t.read(reinterpret_cast<char *>(&num_of_processes), sizeof(int));
		for (int i = 0; i < num_of_processes; i++)
		{
			t.read(reinterpret_cast<char *>(&rank), sizeof(int));
			ranklist.push_back(rank);
		}
		//grp->set_process_list(ranks);
		//exampi::groups.push_back(grp);
		grp = std::make_shared<Group>(ranklist);
		grp->set_group_id(id);
		universe.groups.push_back(grp);
		group_size--;
	}
	//restore communicator
	t.read(reinterpret_cast<char *>(&comm_size), sizeof(int));

	while(comm_size)
	{
		com = std::make_shared<Comm>();
		t.read(reinterpret_cast<char *>(&r), sizeof(int));
		com->set_rank(r);
		t.read(reinterpret_cast<char *>(&p2p), sizeof(int));
		t.read(reinterpret_cast<char *>(&coll), sizeof(int));
		com->set_context(p2p, coll);
		t.read(reinterpret_cast<char *>(&intra), sizeof(bool));
		com->set_is_intra(intra);
		t.read(reinterpret_cast<char *>(&id), sizeof(int));

		auto it = std::find_if(universe.groups.begin(),
		                       universe.groups.end(),
		                       [id](std::shared_ptr<Group> const& i) -> bool {return i->get_group_id() == id;});
		if (it == universe.groups.end())
		{
			return MPIX_TRY_RELOAD;
		}
		else
		{
			com->set_local_group(*it);
		}
		t.read(reinterpret_cast<char *>(&id), sizeof(int));
		it = std::find_if(universe.groups.begin(), universe.groups.end(),
		                  [id](std::shared_ptr<Group> const& i) -> bool {return i->get_group_id() == id;});
		if (it == universe.groups.end())
		{
			return MPIX_TRY_RELOAD;
		}
		else
		{
			com->set_remote_group(*it);
		}
		universe.communicators.push_back(com);
		comm_size--;
	}
	return MPI_SUCCESS;
}

int StagesInterface::cleanup()
{
	return MPI_SUCCESS;
}

int StagesInterface::halt()
{
	return MPI_SUCCESS;
}


}
