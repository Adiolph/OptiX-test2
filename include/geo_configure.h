#include <array>
#include <vector>
#include <optix_world.h>


const unsigned int DOM_NUM_X = 20u;
const unsigned int DOM_NUM_Y = 20u;
const unsigned int DOM_NUM_Z = 20u;
const float DOM_SEP_X = 50.f;
const float DOM_SEP_Y = 50.f;
const float DOM_SEP_Z = 50.f;
const float DOM_RAD = 15.f;
const unsigned int NUM_DOM = 8000u;

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

      float tx = (nx - (DOM_NUM_X - 1) / 2) * DOM_SEP_X;
      float ty = (ny - (DOM_NUM_Y - 1) / 2) * DOM_SEP_Y;
      float tz = (nx - (DOM_NUM_X - 1) / 2) * DOM_SEP_Z;
      transforms.push_back(MakeTranslation(tx, ty, tz));
    }
  }
};
