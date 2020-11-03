#include "geo_create.hh"
#include "read_PTX.hh"

template void createGeometry<true>(Context context);
template void createGeometry<false>(Context context);

template <bool test_geometry>
Material createMaterial(Context context)
{
  Material material = context->createMaterial();
  std::string ptx;
  Program closest_hit;
  if constexpr (test_geometry)
  {
    ptx = read_ptx_file("camera_viewer");
    closest_hit = context->createProgramFromPTXFile(ptx.c_str(), "closest_hit_camera");
  }
  else
  {
    ptx = read_ptx_file("medium_dom");
    closest_hit = context->createProgramFromPTXFile(ptx.c_str(), "closest_hit");
  }
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

template <bool test_geometry>
void createGeometry(Context context)
{
  GeoConfig cfg(NUM_DOM);
  Geometry sphere = createSphere(context);
  Material material = createMaterial<test_geometry>(context);
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

