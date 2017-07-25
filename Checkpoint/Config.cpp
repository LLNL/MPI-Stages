#include "Config.h"
#include <iostream>
#include <cstdlib>
#include <fstream>

Config::Config() {}

Config::Config(std::string configFile) {
	fileName = configFile;
}

void Config::getConfiguration() {
	std::string line;
	std::ifstream file (fileName);
	std::size_t begin;
	if (file.is_open()) {
		while(std::getline(file, line)) {
			begin = line.find_first_not_of(" \t");
			if (begin != std::string::npos) {
				if (line[begin] == '#')
					continue;
				std::size_t end = line.find('=', begin);
				std::string key = line.substr(begin, end - begin);
				std::string value = line.substr(end + 1, line.length() -1);
				if (!key.compare("dirPath")) {
					setDirPath(value);
				}
				else if (!key.compare("localDir")) {
					setLocalDir(value);
				}
			}
		}
	}
	else {
		std::cerr << "Unable to open file.";
	}
}

std::string Config::getDirPath() {
	return dirPath;
}

std::string Config::getLocalDir() {
	return localDir;
}

void Config::setDirPath(std::string path) {
	dirPath = path;
}

void Config::setLocalDir(std::string dir) {
	localDir = dir;
}
