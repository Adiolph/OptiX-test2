#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include "PerRayData_pathtrace.h"

using namespace optix;

rtDeclareVariable(float3, hit_pos, attribute hit_pos, );
rtDeclareVariable(PerRayData_pathtrace, prd, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, len_intersect, rtIntersectionDistance, );

RT_PROGRAM void closest_hit()
{
  float Len_Abs = 30;
  float Len_Sca = 50;
  float len_abs = -Len_Abs * logf(prd.seed);
  float len_sca = -Len_Sca * logf(prd.seed);
  if (len_abs < len_intersect)
  {
    if (len_sca < len_abs)
    {
      prd.command = SCA;
      prd.length = len_sca;
    }
    else
    {
      prd.command = ABS;
    }
  }
  else if(len_sca < len_intersect)
  {
    prd.command = SCA;
    prd.length = len_sca;
  }
  else
  {
    prd.command = HIT;
    prd.hitID = hit_pos.x / 10;
    rtPrintf("hitID: %d, hit_pos: %.3f, %.3f, %.3f \n", prd.hitID, hit_pos.x, hit_pos.y, hit_pos.z);
  }
};
