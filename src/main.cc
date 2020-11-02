#include <optix_world.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "read_PTX.h"
#include "error_check.h"
#include "geo_configure.h"
#include "cherenkov_step.h"
#include <array>

using namespace optix;

const unsigned int NUM_PHOTON = 100u;

void createContext(RTcontext *context);
void createBufferCherenkovStep(RTcontext *context, RTbuffer *cherenkov_step_buf);
void createBufferHit(RTcontext *context, RTbuffer *output_id);
void createGeometry(RTcontext *context, const GeoConfig &cfg);
void createMaterial(RTcontext *context, RTmaterial *material);
void createSphere(RTcontext *context, RTgeometry *sphere);

int main(int argc, char *argv[])
{
  RTcontext context = 0;
  try
  {
    // Primary RTAPI objects
    RTcontext context;
    RTprogram ray_gen_program;
    RTbuffer output_id = 0;
    RTbuffer cherenkov_step = 0;

    // Set up state
    createContext(&context);
    createBufferHit(&context, &output_id);
    createBufferCherenkovStep(&context, &cherenkov_step);
    GeoConfig cfg(NUM_DOM);
    createGeometry(&context, cfg);

    // validate context
    std::cout << "[ context validation " << __FILE__ << ":" << __LINE__ << " ]" << std::endl;
    RT_CHECK_ERROR(rtContextValidate(context));
    std::cout << "[ context validation " << __FILE__ << ":" << __LINE__ << " ]" << std::endl;

    // launch context
    unsigned entry_point_index = 0u;
    RTsize width = NUM_PHOTON;
    RT_CHECK_ERROR(rtContextLaunch1D(context, entry_point_index, width));

    // read output buffer
    void *output_id_ptr;
    RT_CHECK_ERROR(rtBufferMap(output_id, &output_id_ptr));
    int *output_id_data = (int *)output_id_ptr;
    for (int i = 0; i < NUM_PHOTON; i++)
      std::cout << output_id_data[i] << std::endl;
    RT_CHECK_ERROR(rtBufferUnmap(output_id));

    // clean up
    RT_CHECK_ERROR(rtContextDestroy(context));
    return (0);
  }
  SUTIL_CATCH(context)
}

void createContext(RTcontext *context)
{
  // declare programs
  RTprogram ray_gen_program;
  RTprogram exception_program;
  RTprogram miss_program;

  /* variables for ray gen program */
  RTvariable cherenkov_step_var;
  RTbuffer cherenkov_step_buf;

  /* Setup context */
  RT_CHECK_ERROR(rtContextCreate(context));
  RT_CHECK_ERROR(rtContextSetRayTypeCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetEntryPointCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetPrintEnabled(*context, 1));
  RT_CHECK_ERROR(rtContextSetPrintBufferSize(*context, 4096));

  /* Ray generation program */
  std::string ptx = read_ptx_file("gen_cherenkov");
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "gen_cherenkov", &ray_gen_program));
  RT_CHECK_ERROR(rtContextSetRayGenerationProgram(*context, 0, ray_gen_program));

  /* Exception program */
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "exception", &exception_program));
  RT_CHECK_ERROR(rtContextSetExceptionProgram(*context, 0, exception_program));

  /* Miss program */
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "miss", &miss_program));
  RT_CHECK_ERROR(rtContextSetMissProgram(*context, 0, miss_program));
}

void createBufferCherenkovStep(RTcontext *context, RTbuffer *cherenkov_step_buf)
{
  // create buffer
  rtBufferCreate(*context, RT_BUFFER_INPUT, cherenkov_step_buf);
  rtBufferSetFormat(*cherenkov_step_buf, RT_FORMAT_USER);
  rtBufferSetSize1D(*cherenkov_step_buf, 1);

  // fill buffer with data
  void *cherenkov_step_ptr = 0;
  RT_CHECK_ERROR(rtBufferMap(*cherenkov_step_buf, &cherenkov_step_ptr));
  CherenkovStep *cherenkov_step_data = (CherenkovStep *)cherenkov_step_ptr;
  cherenkov_step_data->dir = optix::make_float3(1.f, 0.f, 0.f);
  cherenkov_step_data->pos = optix::make_float3(-500.f, 0.f, 0.f);
  cherenkov_step_data->length = 1000.f;
  cherenkov_step_data->time = 0.f;
  RT_CHECK_ERROR(rtBufferUnmap(*cherenkov_step_buf));

  // declear variable
  RTvariable cherenkov_step_var;
  rtContextDeclareVariable(*context, "cherenkov_step", &cherenkov_step_var);
  rtVariableSetObject(cherenkov_step_var, &cherenkov_step_buf);
}

void createBufferHit(RTcontext *context, RTbuffer *output_id_buf)
{
  rtBufferCreate(*context, RT_BUFFER_OUTPUT, output_id_buf);
  rtBufferSetFormat(*output_id_buf, RT_FORMAT_INT);
  rtBufferSetSize1D(*output_id_buf, NUM_PHOTON);
  RTvariable output_id_var;
  rtContextDeclareVariable(*context, "output_id", &output_id_var);
  rtVariableSetObject(output_id_var, *output_id_buf);
}

void createMaterial(RTcontext *context, RTmaterial *material)
{
  std::string ptx = read_ptx_file("medium_dom");
  std::cout << " createMaterial " << ptx.c_str() << std::endl;

  RTprogram closest_hit;
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "closest_hit", &closest_hit));

  RT_CHECK_ERROR(rtMaterialCreate(*context, material));
  RT_CHECK_ERROR(rtMaterialSetClosestHitProgram(*material, 0, closest_hit));
}

void createSphere(RTcontext *context, RTgeometry *sphere)
{
  std::string ptx = read_ptx_file("sphere");

  RT_CHECK_ERROR(rtGeometryCreate(*context, sphere));
  RT_CHECK_ERROR(rtGeometrySetPrimitiveCount(*sphere, 1u));

  RTprogram bounds;
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "bounds", &bounds));
  RT_CHECK_ERROR(rtGeometrySetBoundingBoxProgram(*sphere, bounds));

  RTprogram intersect;
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "intersect", &intersect));
  RT_CHECK_ERROR(rtGeometrySetIntersectionProgram(*sphere, intersect));

  RTvariable sphere_coor;
  RT_CHECK_ERROR(rtGeometryDeclareVariable(*sphere, "sphere_coor", &sphere_coor));
  float sphere_loc[4] = {0.f, 0.f, 0.f, DOM_RAD};
  RT_CHECK_ERROR(rtVariableSet4fv(sphere_coor, sphere_loc));
}

/*    
createGeometry
--------------

top                      (Group)
    assembly             (Group) 
       xform             (Transform)
           perxform      (GeometryGroup)
              pergi      (GeometryInstance)  
              accel      (Acceleration)   
*/

void createGeometry(RTcontext *context, const GeoConfig &cfg)
{
  RTgeometry sphere;
  createSphere(context, &sphere);

  RTmaterial material;
  createMaterial(context, &material);

  RTacceleration accel;
  RT_CHECK_ERROR(rtAccelerationCreate(*context, &accel));
  RT_CHECK_ERROR(rtAccelerationSetBuilder(accel, "Trbvh"));

  unsigned num_instances = cfg.transforms.size();

  RTacceleration assembly_accel;
  RT_CHECK_ERROR(rtAccelerationCreate(*context, &assembly_accel));
  RT_CHECK_ERROR(rtAccelerationSetBuilder(assembly_accel, "Trbvh"));

  RTgroup assembly;
  RT_CHECK_ERROR(rtGroupCreate(*context, &assembly));
  RT_CHECK_ERROR(rtGroupSetChildCount(assembly, num_instances));
  RT_CHECK_ERROR(rtGroupSetAcceleration(assembly, assembly_accel));

  for (int instance_idx = 0; instance_idx < num_instances; instance_idx++)
  {
    RTtransform xform;
    RT_CHECK_ERROR(rtTransformCreate(*context, &xform));
    int transpose = 0;
    const std::array<float, 16> &arr = cfg.transforms[instance_idx];
    const float *matrix = arr.data();
    const float *inverse = NULL;
    RT_CHECK_ERROR(rtTransformSetMatrix(xform, transpose, matrix, inverse));

    RTgeometryinstance pergi;
    RT_CHECK_ERROR(rtGeometryInstanceCreate(*context, &pergi));
    RT_CHECK_ERROR(rtGeometryInstanceSetGeometry(pergi, sphere));
    RT_CHECK_ERROR(rtGeometryInstanceSetMaterialCount(pergi, 1u));
    RT_CHECK_ERROR(rtGeometryInstanceSetMaterial(pergi, 0, material));

    RTgeometrygroup perxform;
    RT_CHECK_ERROR(rtGeometryGroupCreate(*context, &perxform));
    RT_CHECK_ERROR(rtGeometryGroupSetChildCount(perxform, 1));
    RT_CHECK_ERROR(rtGeometryGroupSetChild(perxform, 0, pergi));
    RT_CHECK_ERROR(rtGeometryGroupSetAcceleration(perxform, accel));

    RT_CHECK_ERROR(rtTransformSetChild(xform, perxform));

    RT_CHECK_ERROR(rtGroupSetChild(assembly, instance_idx, xform));
  }

  RTacceleration top_accel;
  RT_CHECK_ERROR(rtAccelerationCreate(*context, &top_accel));
  RT_CHECK_ERROR(rtAccelerationSetBuilder(top_accel, "Trbvh"));

  RTgroup top;
  RT_CHECK_ERROR(rtGroupCreate(*context, &top));
  RT_CHECK_ERROR(rtGroupSetChildCount(top, 1));
  RT_CHECK_ERROR(rtGroupSetChild(top, 0, assembly));
  RT_CHECK_ERROR(rtGroupSetAcceleration(top, top_accel));

  RTvariable top_object;
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "top_object", &top_object));
  RT_CHECK_ERROR(rtVariableSetObject(top_object, top));
}
