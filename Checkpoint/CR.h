#include "CheckpointData.h"
#include <fstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

/*
 * To serialize data, we used boost serialization library.
 * To install boost serialization library use command <sudo apt install libboost-serialization-dev>
 * To run this library, add library path -L /usr/include and -lboost_serialization to linker
 */
class CR {
private:
	std::ofstream ofs;
	std::ifstream ifs;
	std::string fileName;

	public:
		CR();
		void Checkpoint(CheckpointData data);
		CheckpointData recover(CheckpointData data);
		bool isEmpty();
};
