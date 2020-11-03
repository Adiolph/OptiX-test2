#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <array>

#include "read_PTX.hh"
#include "error_check.h"
#include "geo_create.hh"
#include "cherenkov_step.h"

using namespace optix;

const unsigned int NUM_PHOTON = 200u;

Context createContext();
void createBufferCherenkovStep(Context context);
Buffer createBufferHit(Context context);

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
    createGeometry<false>(context);
    std::cout << "Geometry creation succeed! " << std::endl;

    // validate context
    std::cout << "Validating context... " << std::endl;
    context->validate();
    std::cout << "Context validation succeed! " << std::endl;

    // launch context
    std::cout << "Launching kernel! " << std::endl;
    unsigned entry_point_index = 0u;
    RTsize width = NUM_PHOTON;
    context->launch(entry_point_index, width);

    // read output buffer
    std::cout << "Reading output from buffer... " << std::endl;
    int *output_hit_data = new int[NUM_PHOTON];
    memcpy(output_hit_data, output_hit_buf->map(), NUM_PHOTON * sizeof(int));
    for (int i = 0; i < NUM_PHOTON; i++)
      std::cout << output_hit_data[i] << std::endl;
    output_hit_buf->unmap();
    std::cout << "Finish reading data! " << std::endl;
    delete [] output_hit_data;
    context->destroy();
    std::cout << "OptiX closed! " << std::endl;
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
  context->setPrintBufferSize(10000);

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
  context["cherenkov_steps"]->set(cherenkov_step_buf);
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
