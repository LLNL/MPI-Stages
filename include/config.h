#ifndef __EXAMPI_CONFIG_H
#define __EXAMPI_CONFIG_H

#include <fstream>
#include <iostream>
#include <map>

/* config.h
 * simple inline class for parsing config files into maps
 * used to read MPI options in a clear manner
 * shane farmer auburn university 8/12/2017
 */

/* sf:  a few notes:
 * should probably replace map with unordered_map
 * this is naive w/r/t close and open (trusts the stl)
 * all the stl::string stuff slows this down a good bit; profile!
 * biggest bottleneck is copies of lines for the key/value strings in map; when c++17 is more
 *   widely available use a string_view for this
 * boost string tokenizing would simplify, but not necessarily speed up
 * not all compilers/settings will use move construction for the map in asmap; need to tighten
 */

namespace exampi
{

class Config
{
public:
	static Config &get_instance();

	Config(const Config &c) 			= delete;
	Config &operator=(const Config &c)	= delete;

	std::map<std::string,std::string> asMap();
	const std::string &operator[](const std::string &i);

private:
	std::map<std::string,std::string> dict;

	Config();
	void load(std::string filename);
	void parse(std::string line);
};


}

#endif
