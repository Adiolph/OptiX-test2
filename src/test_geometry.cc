#include <optix_world.h>
#include <sutil/sutil.h>
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
int width = 1024;
int height = 768;

void createContext(RTcontext *context, RTbuffer *output_buffer_obj);
void createGeometry(RTcontext *context, const GeoConfig &cfg);
void createMaterial(RTcontext *context, RTmaterial *material);
void createSphere(RTcontext *context, RTgeometry *sphere);
void printUsageAndExit(const char *argv0);

int main(int argc, char *argv[])
{
  RTcontext context = 0;
  try
  {
    char outfile[512];
    outfile[0] = '\0';
    for (int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
      {
        printUsageAndExit(argv[0]);
      }
      else if (strcmp(argv[i], "--file") == 0 || strcmp(argv[i], "-f") == 0)
      {
        if (i < argc - 1)
        {
          strcpy(outfile, argv[++i]);
        }
        else
        {
          printUsageAndExit(argv[0]);
        }
      }
      else if (strncmp(argv[i], "--dim=", 6) == 0)
      {
        const char *dims_arg = &argv[i][6];
        sutil::parseDimensions(dims_arg, width, height);
      }
      else
      {
        fprintf(stderr, "Unknown option '%s'\n", argv[i]);
        printUsageAndExit(argv[0]);
      }
    }

    /* Process command line args */
    if (strlen(outfile) == 0)
    {
      sutil::initGlut(&argc, argv);
    }

    // Primary RTAPI objects
    RTcontext context;
    RTprogram ray_gen_program;
    RTbuffer output_buffer_obj;

    // Set up state
    createContext(&context, &output_buffer_obj);
    GeoConfig cfg(NUM_DOM);
    createGeometry(&context, cfg);

    // validate context
    std::cout << "[ context validation " << __FILE__ << ":" << __LINE__ << std::endl;
    RT_CHECK_ERROR(rtContextValidate(context));
    std::cout << "] context validation " << __FILE__ << ":" << __LINE__ << std::endl;

    // launch context
    RT_CHECK_ERROR(rtContextLaunch2D(context, 0u, width, height));

    /* Display image */
    if (strlen(outfile) == 0)
    {
      sutil::displayBufferGlut(argv[0], output_buffer_obj);
    }
    else
    {
      sutil::displayBufferPPM(outfile, output_buffer_obj);
    }

    // clean up
    RT_CHECK_ERROR(rtContextDestroy(context));
    return (0);
  }
  SUTIL_CATCH(context)
}

void printUsageAndExit(const char *argv0)
{
  fprintf(stderr, "Usage  : %s [options]\n", argv0);
  fprintf(stderr, "Options: --help | -h             Print this usage message\n");
  fprintf(stderr, "         --file | -f <filename>  Specify file for image output\n");
  fprintf(stderr, "         --dim=<width>x<height>  Set image dimensions; defaults to 1024x768\n");
  exit(1);
}

void createContext(RTcontext *context, RTbuffer *output_buffer_obj)
{
  RTprogram ray_gen_program;
  RTprogram exception_program;
  RTprogram miss_program;
  RTvariable output_buffer;
  RTvariable epsilon;

  /* variables for ray gen program */
  RTvariable eye;
  RTvariable U;
  RTvariable V;
  RTvariable W;
  RTvariable badcolor;

  /* viewing params */
  float hfov, aspect_ratio;

  /* variables for miss program */
  RTvariable bg_color;
  /* Setup context */
  RT_CHECK_ERROR(rtContextCreate(context));
  RT_CHECK_ERROR(rtContextSetRayTypeCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetEntryPointCount(*context, 1));
  RT_CHECK_ERROR(rtContextSetPrintBufferSize(*context, 4096));

  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "output_buffer", &output_buffer));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "scene_epsilon", &epsilon));
  RT_CHECK_ERROR(rtVariableSet1f(epsilon, 1.e-4f));

  /* Render result buffer */
  RT_CHECK_ERROR(rtBufferCreate(*context, RT_BUFFER_OUTPUT, output_buffer_obj));
  RT_CHECK_ERROR(rtBufferSetFormat(*output_buffer_obj, RT_FORMAT_UNSIGNED_BYTE4));
  RT_CHECK_ERROR(rtBufferSetSize2D(*output_buffer_obj, width, height));
  RT_CHECK_ERROR(rtVariableSetObject(output_buffer, *output_buffer_obj));

  /* Ray generation program */
  std::string ptx = read_ptx_file("camera_viewer");
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "ray_gen_camera", &ray_gen_program));
  RT_CHECK_ERROR(rtContextSetRayGenerationProgram(*context, 0, ray_gen_program));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "eye", &eye));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "U", &U));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "V", &V));
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "W", &W));

  optix::float3 cam_eye = {300.f, 0.f, 0.f};
  optix::float3 lookat = {-1.f, 0.f, 0.f};
  optix::float3 up = {0.f, 0.f, 1.f};
  hfov = 60.0f;
  aspect_ratio = (float)width / (float)height;
  optix::float3 camera_u, camera_v, camera_w;
  sutil::calculateCameraVariables(
      cam_eye, lookat, up, hfov, aspect_ratio,
      camera_u, camera_v, camera_w);

  std::cout << "Camera pos: " << cam_eye.x << ", " << cam_eye.y << ", " << cam_eye.z << std::endl;
  std::cout << "U: " << camera_u.x << ", " << camera_u.y << ", " << camera_u.z << std::endl;
  std::cout << "V: " << camera_v.x << ", " << camera_v.y << ", " << camera_v.z << std::endl;
  std::cout << "W: " << camera_w.x << ", " << camera_w.y << ", " << camera_w.z << std::endl;

  RT_CHECK_ERROR(rtVariableSet3fv(eye, &cam_eye.x));
  RT_CHECK_ERROR(rtVariableSet3fv(U, &camera_u.x));
  RT_CHECK_ERROR(rtVariableSet3fv(V, &camera_v.x));
  RT_CHECK_ERROR(rtVariableSet3fv(W, &camera_w.x));

  /* Exception program */
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "bad_color", &badcolor));
  RT_CHECK_ERROR(rtVariableSet3f(badcolor, 1.0f, 0.0f, 1.0f));
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "exception_camera", &exception_program));
  RT_CHECK_ERROR(rtContextSetExceptionProgram(*context, 0, exception_program));

  /* Miss program */
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "miss_camera", &miss_program));
  RT_CHECK_ERROR(rtProgramDeclareVariable(miss_program, "bg_color", &bg_color));
  RT_CHECK_ERROR(rtVariableSet3f(bg_color, .3f, 0.1f, 0.2f));
  RT_CHECK_ERROR(rtContextSetMissProgram(*context, 0, miss_program));
}

void createMaterial(RTcontext *context, RTmaterial *material)
{
  std::string ptx = read_ptx_file("camera_viewer");
  std::cout << " createMaterial " << ptx.c_str() << std::endl;

  RTprogram closest_hit;
  RT_CHECK_ERROR(rtProgramCreateFromPTXFile(*context, ptx.c_str(), "closest_hit_camera", &closest_hit));

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
    RT_CHECK_ERROR(rtGeometryGroupSetChildCount(perxform, 1u));
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
  RT_CHECK_ERROR(rtGroupSetChildCount(top, 1u));
  RT_CHECK_ERROR(rtGroupSetChild(top, 0, assembly));
  RT_CHECK_ERROR(rtGroupSetAcceleration(top, top_accel));

  RTvariable top_object;
  RT_CHECK_ERROR(rtContextDeclareVariable(*context, "top_object", &top_object));
  RT_CHECK_ERROR(rtVariableSetObject(top_object, top));
}
