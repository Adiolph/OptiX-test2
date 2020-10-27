#include <sstream>
#include <cstring>
#include <iostream>
#include "config.h"

std::string PTXPath(const std::string install_prefix, const std::string file_name)
{
  return install_prefix + "/ptx/" + file_name + ".ptx";
}

std::string read_ptx_file(const std::string file_name)
{
  std::string ptx = PTXPath(CMAKE_INSTALL_PREFIX, file_name);
  std::cout << "Create ptx source: " << ptx << std::endl;
  return ptx;
}