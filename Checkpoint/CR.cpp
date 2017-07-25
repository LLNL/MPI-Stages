#include "CR.h"
#include "Config.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


CR::CR() {
	Config config("config.txt");
	config.getConfiguration();
	std::string directory = config.getDirPath() + "/" + config.getLocalDir();
	struct stat sb;
	if (!(stat(directory.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))) {
		const int dir_err = mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (-1 == dir_err)
		{
			printf("Error creating directory!");
			exit(1);
		}
	}
	fileName = config.getDirPath() + "/" + config.getLocalDir() + "/" + "check.txt";
}

void CR::Checkpoint(CheckpointData data) {
	std::ofstream ofs(fileName.c_str());
	boost::archive::text_oarchive ar(ofs);
	ar& data;
	ofs.close();
}

CheckpointData CR::recover(CheckpointData data) {
	std::ifstream ifs(fileName.c_str());
	boost::archive::text_iarchive ar(ifs);
	ar& data;
	ifs.close();
	return data;
}

bool CR::isEmpty() {
	std::ifstream ifs(fileName.c_str());
	if (ifs.peek() == std::ifstream::traits_type::eof()) {
		ifs.close();
		return true;
	}
	ifs.close();
	return false;
}
