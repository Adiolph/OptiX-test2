#include <array>
#include <vector>
#include <optix_world.h>
#include <iostream>

const unsigned int DOM_NUM_X = 4u;
const unsigned int DOM_NUM_Y = 4u;
const unsigned int DOM_NUM_Z = 4u;
const float DOM_SEP_X = 50.f;
const float DOM_SEP_Y = 50.f;
const float DOM_SEP_Z = 50.f;
const float DOM_RAD = 15.f;
const unsigned int NUM_DOM = DOM_NUM_X*DOM_NUM_Y*DOM_NUM_Z;

struct GeoConfig
{
  typedef std::array<float, 16> Transform_t;
  std::vector<Transform_t> transforms;
  static Transform_t MakeTranslation(float tx, float ty, float tz)
  {
    return {1.f, 0.f, 0.f, tx,
            0.f, 1.f, 0.f, ty,
            0.f, 0.f, 1.f, tz,
            0.f, 0.f, 0.f, 1.f};
  }

  GeoConfig(unsigned num)
  {
    for (int i = 0; i < num; i++)
    {
      int nx = static_cast<int>(i / DOM_NUM_Y / DOM_NUM_Z);
      int ny = static_cast<int>((i - (DOM_NUM_Y * DOM_NUM_Z) * nx) / DOM_NUM_Z);
      int nz = i - (DOM_NUM_Y * DOM_NUM_Z) * nx - DOM_NUM_Z * ny;

      float tx = (nx - (DOM_NUM_X - 1.f) / 2.f) * DOM_SEP_X;
      float ty = (ny - (DOM_NUM_Y - 1.f) / 2.f) * DOM_SEP_Y;
      float tz = (nz - (DOM_NUM_Z - 1.f) / 2.f) * DOM_SEP_Z;
      transforms.push_back(MakeTranslation(tx, ty, tz));
      // std::cout << "nxnynz: " << nx << ", " << ny << ", " << nz << std::endl;
      // std::cout << "txtytz: " << tx << ", " << ty << ", " << tz << std::endl;
    }
  }
};
