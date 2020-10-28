#include <optix_world.h>
#include "PerRayData_pathtrace.h"

using namespace optix;

rtDeclareVariable(float3, hit_pos, attribute hit_pos, );
rtDeclareVariable(PerRayData_pathtrace, prd, rtPayload, );

RT_PROGRAM void closest_hit()
{
  prd.hitID = __float2int_rd(hit_pos.x);
  rtPrintf("hitID: %d, hit_pos: %.3f, %.3f, %.3f \n", prd.hitID, hit_pos.x, hit_pos.y, hit_pos.z);
};
