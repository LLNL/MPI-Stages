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

#include "configuration.h"
#include "json.hpp"

namespace exampi
{

Configuration &Configuration::get_instance()
{
	static Configuration instance;

	return instance;
}

Configuration::Configuration()
{
	// load configuration file from environment variable
	std::string filename = std::string(std::getenv("EXAMPI_CONFIG_FILE"));
	debug("config loading " << filename);

	load(filename);
}

void Configuration::load(std::string filename)
{
	// read configuration file
	std::ifstream file(filename, std::ifstream::in);

	// check valid file
	if(!file.is_open())
	{
		throw std::runtime_error("Configuration could not be loaded");
	}

	// read in data
	file >> data;

	debug("json data: " << data);
}

const nlohmann::json &Configuration::operator[](std::string rank)
{
	return data[rank];
}

} //exampi
