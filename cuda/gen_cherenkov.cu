#include <optix_world.h>
#include "random.h"  // OptiX random header file in SDK/cuda/random.h
#include "PerRayData_pathtrace.h"
#include "cherenkov_step.h"
#include "rotateUz.h"

using namespace optix;

#define PI 3.1415926

rtDeclareVariable(unsigned int, random_seed, , );  // the random seed of the kernel
rtDeclareVariable(rtObject, top_object, , );  // group object
rtDeclareVariable(uint, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint, launch_dim, rtLaunchDim, );
rtBuffer<CherenkovStep> cherenkov_steps;  // the step that can emit Cherenkov photons
rtBuffer<uint, 1> output_id;  // record the id of dom that photon hit, 0 if no hit

__device__ float3 gen_CK_dir(float costh, float sinth, unsigned int &random_seed);

RT_PROGRAM void gen_cherenkov()
{
  const CherenkovStep cherenkov_step = cherenkov_steps[0];
  PerRayData_pathtrace prd;
  prd.seed = tea<4>(launch_index, random_seed);
  prd.command = 0;
  prd.hitID = 0;
  float3 ray_origin = cherenkov_step.pos + cherenkov_step.length * rnd(prd.seed) * cherenkov_step.dir;
  // TODO: add wave length depedented cherenkov angle
  float3 ray_dir = gen_CK_dir(sqrtf(3)/2, 0.5, prd.seed);
  rotateUz(ray_dir, cherenkov_step.dir);
  rtPrintf("// point_source  ray_direction: (%.3f %.3f %.3f), ray_origin: (%.3f %.3f %.3f) \n",
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
        rotateUz(ray_dir, SCA_dir);
        ray = make_Ray(ray_origin, ray_dir, 0, 0.01, RT_DEFAULT_MAX);
        prd.length = 0;
        prd.command = 0;
        rtTrace(top_object, ray, prd);
        break;
      }
      case ABS:
      {
        count++;
        ray_origin = cherenkov_step.pos + cherenkov_step.length * rnd(prd.seed) * cherenkov_step.dir;
        ray_dir = gen_CK_dir(sqrtf(3)/2, 0.5, prd.seed);
        rotateUz(ray_dir, cherenkov_step.dir);
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

__device__ float3 gen_CK_dir(float cosTheta, float sinTheta, unsigned int &random_seed)
{
    float3 dir;
    float phi = rnd(random_seed) * 2 * M_PI;
    float sinPhi, cosPhi;
    sincosf(phi, &sinPhi, &cosPhi);
    dir.x = cosPhi * sinTheta;
    dir.y = sinPhi * sinTheta;
    dir.z = cosTheta;
    return dir;
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
