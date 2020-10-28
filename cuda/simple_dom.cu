#include <optix_world.h>
#include "PerRayData_pathtrace.h"

using namespace optix;

rtDeclareVariable(float3, hit_pos, attribute hit_pos, );
rtDeclareVariable(PerRayData_pathtrace, prd, rtPayload, );

RT_PROGRAM void closest_hit()
{
  prd.hitID = hit_pos.x / 10;
};
