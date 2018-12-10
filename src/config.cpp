#include "config.h"

#include "ExaMPI.h"

namespace exampi
{

Config& Config::get_instance() {
	static Config instance;
	
	return instance;
}

Config::Config()
{
	// load configuration file from environment variable
	std::string filename = std::string(std::getenv("EXAMPI_CONFIG_FILE"));
	debugpp("config loading " << filename);
	load(filename);	
}

void Config::load(std::string filename)
{
	// read configuration file
	std::ifstream file(filename, std::ifstream::in);

	if(!file.is_open())
		std::cout << "CONFIG NOT LOADED" << std::endl;

	std::string next;
	while(std::getline(file, next))
		parse(next);
}

void Config::parse(std::string line)
{
	std::size_t delim = line.find_first_of(":");
	std::string key = line.substr(0, delim);
	std::string val = line.substr(delim + 1);

	dict[key] = val;
}

std::map<std::string, std::string> Config::asMap()
{
	return dict;
}

const std::string& Config::operator[](const std::string &i)
{
	return dict[i];
}

}; //exampi
