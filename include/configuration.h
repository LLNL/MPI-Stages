#ifndef __EXAMPI_CONFIG_H
#define __EXAMPI_CONFIG_H

#include <fstream>
#include <iostream>
#include <map>

#include "json.hpp"
#include "debug.h"

namespace exampi
{

class Configuration
{
public:
	static Configuration &get_instance();

	Configuration(const Configuration &c) 				= delete;
	Configuration &operator=(const Configuration &c)	= delete;

	const nlohmann::json &operator[](std::string rank);

private:
	nlohmann::json data;

	Configuration();

	void load(std::string filename);
};

}

#endif
