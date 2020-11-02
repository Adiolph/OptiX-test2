#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
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

Context createContext();
void createBufferCherenkovStep(Context context);
Buffer createBufferHit(Context context);
Geometry createSphere(Context context);
Material createMaterial(Context context);
void createGeometry(Context context, const GeoConfig &cfg);

int main(int argc, char *argv[])
{
  try
  {
    // Creating context
    std::cout << "Creating context... " << std::endl;
    Context context = createContext();
    std::cout << "Context creation succeed! " << std::endl;

    // Creating buffer
    std::cout << "Creating buffer... " << std::endl;
    Buffer output_hit_buf = createBufferHit(context);
    createBufferCherenkovStep(context);
    std::cout << "Buffer creation succeed! " << std::endl;

    std::cout << "Creating geometry... " << std::endl;
    GeoConfig cfg(NUM_DOM);
    createGeometry(context, cfg);
    std::cout << "Geometry creation succeed! " << std::endl;

    // validate context
    std::cout << "Validating context... " << std::endl;
    context->validate();
    std::cout << "Context validation succeed! " << std::endl;

    // launch context
    unsigned entry_point_index = 0u;
    RTsize width = NUM_PHOTON;
    context->launch(entry_point_index, width);

    // read output buffer
    int *output_id_data;
    memcpy(output_id_data, output_hit_buf->map(), NUM_PHOTON * sizeof(int));
    for (int i = 0; i < NUM_PHOTON; i++)
      std::cout << output_id_data[i] << std::endl;
    output_hit_buf->unmap();
    context->destroy();
  }
  catch (const Exception &e)
  {
    reportErrorMessage(e.getErrorString().c_str());
    exit(1);
  }

  return (0);
}

Context createContext()
{
  Context context = Context::create();

  /* Setup context */
  context->setRayTypeCount(1);
  context->setEntryPointCount(1);
  context->setPrintEnabled(1);
  context->setPrintBufferSize(4096);

  /* Ray generation program */
  std::string ptx = read_ptx_file("gen_cherenkov");
  Program ray_gen_program = context->createProgramFromPTXFile(ptx.c_str(), "gen_cherenkov");
  context->setRayGenerationProgram(0, ray_gen_program);

  /* Exception program */
  Program exception_program = context->createProgramFromPTXFile(ptx.c_str(), "exception");
  context->setExceptionProgram(0, exception_program);

  /* Miss program */
  Program miss_program = context->createProgramFromPTXFile(ptx.c_str(), "miss");
  context->setMissProgram(0, miss_program);
  return context;
}

void createBufferCherenkovStep(Context context)
{
  Buffer cherenkov_step_buf = context->createBuffer(RT_BUFFER_INPUT);
  cherenkov_step_buf->setFormat(RT_FORMAT_USER);
  cherenkov_step_buf->setElementSize(sizeof(CherenkovStep));
  cherenkov_step_buf->setSize(1);

  CherenkovStep *cherenkov_step_data = reinterpret_cast<CherenkovStep *>(cherenkov_step_buf->map());
  cherenkov_step_data->pos = optix::make_float3(-500.f, 0.f, 0.f);
  cherenkov_step_data->dir = optix::make_float3(1.f, 0.f, 0.f);
  cherenkov_step_data->length = 1000.f;
  cherenkov_step_data->time = 0.f;
  cherenkov_step_data->num_photon = NUM_PHOTON;
  cherenkov_step_buf->unmap();
  context["cherenkov_step"]->set(cherenkov_step_buf);
}

Buffer createBufferHit(Context context)
{
  Buffer output_id_buf = context->createBuffer(RT_BUFFER_OUTPUT);
  output_id_buf->setFormat(RT_FORMAT_INT);
  output_id_buf->setSize(NUM_PHOTON);
  Variable output_id_var = context["output_id"];
  output_id_var->set(output_id_buf);
  return output_id_buf;
}

Material createMaterial(Context context)
{
  Material material = context->createMaterial();
  std::string ptx = read_ptx_file("medium_dom");
  Program closest_hit = context->createProgramFromPTXFile(ptx.c_str(), "closest_hit");
  material->setClosestHitProgram(0, closest_hit);
  return material;
}

Geometry createSphere(Context context)
{
  Geometry sphere = context->createGeometry();
  sphere->setPrimitiveCount(1u);

  std::string ptx = read_ptx_file("sphere");
  Program bounds = context->createProgramFromPTXFile(ptx.c_str(), "bounds");
  sphere->setBoundingBoxProgram(bounds);

  Program intersect = context->createProgramFromPTXFile(ptx.c_str(), "intersect");
  sphere->setIntersectionProgram(intersect);

  Variable sphere_coor_var = context["sphere_coor"];
  float sphere_coor_data[4] = {0.f, 0.f, 0.f, DOM_RAD};
  sphere_coor_var->set4fv(sphere_coor_data);
  return sphere;
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

void createGeometry(Context context, const GeoConfig &cfg)
{
  Geometry sphere = createSphere(context);
  Material material = createMaterial(context);
  Acceleration accel = context->createAcceleration("Trbvh");
  Acceleration assembly_accel = context->createAcceleration("Trbvh");
  unsigned num_instances = cfg.transforms.size();
  Group assembly = context->createGroup();
  assembly->setChildCount(num_instances);
  assembly->setAcceleration(assembly_accel);

  for (int instance_idx = 0; instance_idx < num_instances; instance_idx++)
  {
    Transform xform = context->createTransform();
    int transpose = 0;
    const std::array<float, 16> &arr = cfg.transforms[instance_idx];
    const float *matrix = arr.data();
    const float *inverse = NULL;
    xform->setMatrix(transpose, matrix, inverse);

    GeometryInstance pergi = context->createGeometryInstance();
    pergi->setGeometry(sphere);
    pergi->setMaterialCount(1u);
    pergi->setMaterial(0, material);

    GeometryGroup perxform = context->createGeometryGroup();
    perxform->setChildCount(1u);
    perxform->setChild(0, pergi);
    perxform->setAcceleration(accel);

    xform->setChild(perxform);
    assembly->setChild(instance_idx, xform);
  }
  Group top = context->createGroup();
  Acceleration top_accel = context->createAcceleration("Trbvh");
  top->setChildCount(1u);
  top->setChild(0, assembly);
  top->setAcceleration(top_accel);

  Variable top_object = context["top_object"];
  top_object->set(top);
}
