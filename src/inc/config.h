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
  private:
    std::map<std::string,std::string> dict;
    void parse(std::string line)
    {
      std::size_t delim = line.find_first_of(":");
      std::string key = line.substr(0, delim);
      std::string val = line.substr(delim+1);
      std::cout << "Config:  Adding " << key << " as " << val << std::endl;
      dict[key] = val;
    }
  public:
    Config() : dict()
    {
    }

    Config(const Config &c)
    {
      dict = c.dict;
    }

    void Load(std::string filename)
    {
      std::ifstream file(filename, std::ifstream::in);
      std::string next;
      while(std::getline(file, next))
        parse(next);

    }

    std::map<std::string,std::string> AsMap()
    {
      return dict;
    }

    const std::string& operator[](const std::string &i)
    {
      return dict[i];
    }
};


}




//fndef __EXAMPI_CONFIG_H
#endif
