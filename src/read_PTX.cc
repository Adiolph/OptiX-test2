#include <sstream>
#include <cstring>
#include <iostream>
#include "config.h"

const char *PTXPath(const char *install_prefix, const char *file_name)
{
  std::stringstream ss;
  ss << install_prefix
     << "/ptx/"
     << file_name
     << ".ptx";
  std::string path = ss.str();
  return strdup(path.c_str());
}

const char *read_ptx_file(const char *file_name)
{
  const char *ptx = PTXPath(CMAKE_INSTALL_PREFIX, "point_source");
  std::cout << "Create ptx source :" << ptx << std::endl;
  return ptx;
}