#pragma once

#include <optix.h>

#define HIT 1
#define SCA 2
#define ABS 3

struct PerRayData_pathtrace
{
  unsigned int seed;
  unsigned int command;
  float length;
  unsigned int hitID;
};

