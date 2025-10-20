#include <graphics/wall.h>
#include <raymath.h>

Wall::Wall(SurfaceType type, const Vector3& normal, float distance, 
     const std::array<Vector3, 4>& vertices)
     : type(type), normal(Vector3Normalize(normal)), xyz_distance(distance), vertices(vertices) 
{}

SurfaceType Wall::get_type() const { return type; }
Vector3 Wall::get_normal() const { return normal; }
const std::array<Vector3, 4>& Wall::get_vertices() const { return vertices; }

Vector3 Wall::get_center() const
{
  Vector3 center = {0, 0, 0};
  for (const auto& vertex : vertices) {
    center = center + vertex;
  }
  return Vector3Scale(center, 1.0f / vertices.size());
}

float Wall::calc_distance_to_point(const Vector3& point) const 
{ 
  // Расстояние от точки до плоскости: |Ax + By + Cz + D| / sqrt(A^2 + B^2 + C^2)
  // Где (A,B,C) - нормаль, D = -расстояние
  return fabs(Vector3DotProduct(point, normal) - xyz_distance);
}

