#include <optix_world.h>
#include <sutil/sutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "read_PTX.hh"
#include "error_check.h"
#include "geo_configure.hh"
#include "geo_create.hh"
#include <array>

using namespace optix;

const unsigned int NUM_PHOTON = 100u;
int width = 1024;
int height = 768;

Context createContext(Buffer &output_buffer);
void printUsageAndExit(const char *argv0);

int main(int argc, char *argv[])
{
  Buffer output_buffer;
  Context context = createContext(output_buffer);
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
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
    std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
    /* Process command line args */
    if (strlen(outfile) == 0)
    {
      sutil::initGlut(&argc, argv);
    }

    // Set up state
    std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
    GeoConfig cfg(NUM_DOM);
    std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
    createGeometry<true>(context, cfg);
    std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 

    // validate context
    std::cout << "Validating context... " << std::endl;
    context->validate();
    std::cout << "Context validation succeed! " << std::endl;

    // launch context
    context->launch(0u, width, height);

    /* Display image */
    if (strlen(outfile) == 0)
    {
      sutil::displayBufferGlut(argv[0], output_buffer);
    }
    else
    {
      sutil::displayBufferPPM(outfile, output_buffer);
    }

    // clean up
    context->destroy();
  }
  catch (const Exception &e)
  {
    reportErrorMessage(e.getErrorString().c_str());
    exit(1);
  }

  return (0);
}

void printUsageAndExit(const char *argv0)
{
  fprintf(stderr, "Usage  : %s [options]\n", argv0);
  fprintf(stderr, "Options: --help | -h             Print this usage message\n");
  fprintf(stderr, "         --file | -f <filename>  Specify file for image output\n");
  fprintf(stderr, "         --dim=<width>x<height>  Set image dimensions; defaults to 1024x768\n");
  exit(1);
}

Context createContext(Buffer &output_buffer)
{
  /* Setup context */
  Context context = Context::create();
  context->setRayTypeCount(1u);
  context->setEntryPointCount(1u);
  context->setPrintBufferSize(4096);
  std::string ptx = read_ptx_file("camera_viewer");

  /* variables for ray gen program */
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  Variable eye = context["eye"];
  Variable U = context["U"];
  Variable V = context["V"];
  Variable W = context["W"];
  Variable epsilon = context["scene_epsilon"];
  float hfov, aspect_ratio;
  optix::float3 camera_eye = {350.f, 0.f, 0.f};
  optix::float3 lookat = {-1.f, 0.f, 0.f};
  optix::float3 up = {0.f, 0.f, 1.f};
  hfov = 60.0f;
  aspect_ratio = (float)width / (float)height;
  float epsilon_data = 1.e-4f;
  optix::float3 camera_u, camera_v, camera_w;
  sutil::calculateCameraVariables(
      camera_eye, lookat, up, hfov, aspect_ratio,
      camera_u, camera_v, camera_w);

  // print ray camera set up variables
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  std::cout << "Camera pos: " << camera_eye.x << ", " << camera_eye.y << ", " << camera_eye.z << std::endl;
  std::cout << "U: " << camera_u.x << ", " << camera_u.y << ", " << camera_u.z << std::endl;
  std::cout << "V: " << camera_v.x << ", " << camera_v.y << ", " << camera_v.z << std::endl;
  std::cout << "W: " << camera_w.x << ", " << camera_w.y << ", " << camera_w.z << std::endl;

  // ray generation program
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  Program ray_gen = context->createProgramFromPTXFile(ptx.c_str(), "ray_gen_camera");
  context->setRayGenerationProgram(0, ray_gen);
  eye->set3fv(&camera_eye.x);
  U->set3fv(&camera_u.x);
  V->set3fv(&camera_v.x);
  W->set3fv(&camera_w.x);
  epsilon->set1fv(&epsilon_data);

  // exception program set up
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  Program exception_program = context->createProgramFromPTXFile(ptx.c_str(), "exception_camera");
  context->setExceptionProgram(0, exception_program);
  Variable bad_color = context["bad_color"];
  float3 bad_color_data = {1.f, 0.f, 1.f};
  bad_color->set3fv(&bad_color_data.x);

  // miss program set up
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  Program miss_program = context->createProgramFromPTXFile(ptx.c_str(), "miss_camera");
  context->setMissProgram(0, miss_program);
  Variable bg_color = context["bg_color"];
  float3 bg_color_data = {.3f, 0.1f, 0.2f};
  bg_color->set3fv(&bg_color_data.x);

  // set up output buffer
  std::cout << "At: " << __FILE__ << ": " << __LINE__ << std::endl ; 
  output_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height);
  Variable output_var = context["output_buffer"];
  output_var->set(output_buffer);

  return context;
}