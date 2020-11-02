#pragma once

#include <optix.h>

using namespace optix;

struct CherenkovStep
{
  optix::float3 pos;
  optix::float3 dir;
  float length;
  float time;
  unsigned int num_photon;
};
