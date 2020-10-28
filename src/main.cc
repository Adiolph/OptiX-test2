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
#include <array>

using namespace optix;

const unsigned int NUM_PHOTON = 100u;

void createContext(RTcontext *context);
void createHitBuffer(RTcontext *context, RTbuffer *output_id);
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
    RTbuffer output_id;

    // Set up state
    createContext(&context);
    createHitBuffer(&context, &output_id);
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
  RTvariable source_pos;

  /* Setup context */
  RT_CHECK_ERROR(rtContextCreate(context));
  RT_CHECK_ERROR(rtContextSetRayTypeCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetEntryPointCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetPrintEnabled(*context, 1));
  RT_CHECK_ERROR(rtContextSetPrintBufferSize(*context, 4096));

  /* Ray generation program */
  std::string ptx = read_ptx_file("point_source");
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "point_source", &ray_gen_program));
  RT_CHECK_ERROR(rtContextSetRayGenerationProgram(*context, 0, ray_gen_program));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "source_pos", &source_pos));
  rtVariableSet3f(source_pos, 0.f, 0.f, 16.f);

  /* Exception program */
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "exception", &exception_program));
  RT_CHECK_ERROR(rtContextSetExceptionProgram(*context, 0, exception_program));

  /* Miss program */
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "miss", &miss_program));
  RT_CHECK_ERROR(rtContextSetMissProgram(*context, 0, miss_program));
}

void createHitBuffer(RTcontext *context, RTbuffer *output_id)
{
  rtBufferCreate(*context, RT_BUFFER_OUTPUT, output_id);
  rtBufferSetFormat(*output_id, RT_FORMAT_INT);
  rtBufferSetSize1D(*output_id, NUM_PHOTON);
  RTvariable output_id_var;
  rtContextDeclareVariable(*context, "output_id", &output_id_var);
  rtVariableSetObject(output_id_var, *output_id);
}

void createMaterial(RTcontext *context, RTmaterial *material)
{
  std::string ptx = read_ptx_file("simple_dom");
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
    int transpose = 1;
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
