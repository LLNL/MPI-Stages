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
