#pragma once

#include <optix.h>

using namespace optix;

struct CherenkovStep
{
  float3 pos;
  float3 dir;
  float length;
  float time;
  unsigned int num_photon;
};
