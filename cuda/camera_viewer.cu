#include <optix_world.h>
using namespace optix;

struct PerRayData_camera
{
  float3 result;
  float  importance;
  int    depth;
};

rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float3,        bg_color, , );
rtDeclareVariable(float,         scene_epsilon, , );
rtBuffer<uchar4, 2>              output_buffer;
rtDeclareVariable(rtObject,      top_object, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 

// copied from OptiX SDK helpers.h
// Convert a float3 in [0,1)^3 to a uchar4 in [0,255]^4 -- 4th channel is set to 255
#ifdef __CUDACC__
static __device__ __inline__ optix::uchar4 make_color(const optix::float3& c)
{
    return optix::make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                               static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                               static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                               255u);                                                 /* A */
}
#endif

RT_PROGRAM void ray_gen_camera()
{
  float2 d = make_float2(launch_index) / make_float2(launch_dim) * 2.f - 1.f;
  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);
  
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, 0u, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData_camera prd;
  prd.importance = 1.f;
  prd.depth = 0;

  rtTrace(top_object, ray, prd ) ;
  output_buffer[launch_index] = make_color( prd.result );
}

RT_PROGRAM void exception_camera()
{
  rtPrintExceptionDetails();
  output_buffer[launch_index] = make_color( bad_color );
}

rtDeclareVariable(PerRayData_camera, prd, rtPayload, );

RT_PROGRAM void miss_camera()
{
  prd.result = bg_color;
}

RT_PROGRAM void closest_hit_camera()
{
  prd.result = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal))*0.5f + 0.5f;
}
