#include "config.h"

namespace exampi
{

Config& Config::get_instance() {
	static Config instance;
	
	return instance;
}

Config::Config() : dict()
{
}

Config::~Config()
{
}

Config *Config::get_instance()
{
	if (instance == 0)
	{
		instance = new Config();
	}
	return instance;
}

void Config::load(std::string filename)
{
	std::ifstream file(filename, std::ifstream::in);
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

void Config::destroy_instance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

}; //exampi
