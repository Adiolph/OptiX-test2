#include <optix_world.h>
#include "random.h"  // OptiX random header file in SDK/cuda/random.h
#include "PerRayData_pathtrace.h"
#include "cherenkov_step.h"

using namespace optix;

#define PI 3.1415926

rtDeclareVariable(unsigned int, random_seed, , );  // the random seed of the kernel
rtDeclareVariable(rtObject, top_object, , );  // group object
rtDeclareVariable(uint, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint, launch_dim, rtLaunchDim, );
rtBuffer<CherenkovStep> cherenkov_steps;  // the step that can emit Cherenkov photons
rtBuffer<uint, 1> output_id;  // record the id of dom that photon hit, 0 if no hit

__device__ float3 gen_CK_dir(float costh, float sinth, unsigned int &random_seed);
__device__ float3 rotate_by_axis(float3 axis, float3 vec);

RT_PROGRAM void gen_cherenkov()
{
  CherenkovStep cherenkov_step = cherenkov_steps[0];
  PerRayData_pathtrace prd;
  prd.seed = tea<4>(launch_index, random_seed);
  prd.command = 0;
  prd.hitID = 0;
  float3 ray_origin = cherenkov_step.pos + cherenkov_step.length * rnd(prd.seed) * cherenkov_step.dir;
  float3 CK_dir = gen_CK_dir(sqrtf(3)/2, 0.5, prd.seed);
  float3 ray_dir = rotate_by_axis(cherenkov_step.dir, CK_dir);
  rtPrintf("//point_source  ray_direction (%.3f %.3f %.3f) ray_origin (%.3f %.3f %.3f)  \n", 
  ray_dir.x, ray_dir.y, ray_dir.z,
  ray_origin.x, ray_origin.y, ray_origin.z
  );  

  Ray ray = make_Ray(ray_origin, ray_dir, 0, 0.01, RT_DEFAULT_MAX);
  rtTrace(top_object, ray, prd);

  int flag_continue = 1;
  int count = 0;
  while(flag_continue)
  {
    // test scattering, absorption and hit.
    switch(prd.command)
    {
      // scattering: generate a new direction according to original direction, re-emit rays
      case SCA:
      {
        float3 SCA_dir = gen_CK_dir(sqrtf(1-0.01), 0.1, prd.seed);
        ray_origin = ray_origin + prd.length * ray_dir;
        ray_dir = rotate_by_axis(ray_dir, SCA_dir);
        ray = make_Ray(ray_origin, ray_dir, 0, 0.01, RT_DEFAULT_MAX);
        prd.length = 0;
        prd.command = 0;
        rtTrace(top_object, ray, prd);
        break;
      }
      case ABS:
      {
        count++;
        CK_dir = gen_CK_dir(sqrtf(3)/2, 0.5, prd.seed);
        ray_dir = rotate_by_axis(cherenkov_step.dir, CK_dir);
        ray_origin = cherenkov_step.pos + cherenkov_step.length * rnd(prd.seed) * cherenkov_step.dir;
        ray = make_Ray(ray_origin, ray_dir, 0, 0.01, RT_DEFAULT_MAX);
        prd.length = 0;
        prd.command = 0;
        rtTrace(top_object, ray, prd);
        break;
      }
      case HIT:
      {
        output_id[launch_index] = prd.hitID;
        flag_continue = 0;
        break;
      }
    }
  }
  rtPrintf("hitID: %d, num_photon: %d. \n", prd.hitID, count);
}

__device__ float3 gen_CK_dir(float costh, float sinth, unsigned int &random_seed)
{
    float3 dir;
    double phi = rnd(random_seed) * 2 * M_PI;
    dir.x = cos(phi) * sinth;
    dir.y = sin(phi) * sinth;
    dir.z = costh;
    return dir;
}

__device__ float3 rotate_by_axis(float3 axis, float3 vec)
{
  float2 proj = make_float2(-axis.y, axis.x);
  proj /= sqrtf(axis.x*axis.x + axis.y*axis.y);
  float3 vec_new;
  // rotate vec by proj with angle of arccos(axis.z)
  vec_new.x = (axis.z+(1-axis.z)*proj.x*proj.x) * vec.x 
              + ((1-axis.z)*proj.x*proj.y) * vec.y 
              + (sqrtf(1-axis.z*axis.z)*proj.y) * vec.z;
  vec_new.y = ((1-axis.z)*proj.x*proj.y) * vec.x 
              + (axis.z+(1-axis.z)*proj.y*proj.y) * vec.y 
              + (-sqrtf(1-axis.z*axis.z)*proj.x) * vec.z;
  vec_new.z = (-sqrtf(1-axis.z*axis.z)*proj.y) * vec.x 
              + (sqrtf(1-axis.z*axis.z)*proj.x) * vec.y 
              + (axis.z) * vec.z;
  return vec_new;
}

rtDeclareVariable(PerRayData_pathtrace, prd, rtPayload, );

RT_PROGRAM void exception()
{
  rtPrintExceptionDetails();
}

RT_PROGRAM void miss()
{
  float Len_Abs = 30;
  float Len_Sca = 50;
  float len_abs = -Len_Abs * logf(rnd(prd.seed));
  float len_sca = -Len_Sca * logf(rnd(prd.seed));
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
