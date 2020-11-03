#pragma once

#include <optix_world.h>
#include <optixu/optixpp_namespace.h>
#include "geo_configure.hh"

using namespace optix;

template<bool test_geometry>
Geometry createSphere(Context context);

Material createMaterial(Context context);

template<bool test_geometry>
void createGeometry(Context context);