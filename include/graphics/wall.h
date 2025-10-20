#ifndef WALL_H
#define WALL_H

#include <raylib.h>
#include <array>

enum class SurfaceType
{
  FLOOR,
  CEILING,
  WALL_FRONT,
  WALL_BACK,
  WALL_LEFT,
  WALL_RIGHT
};

class Wall 
{
private:
  SurfaceType             type;
  Vector3                 normal;       // Нормаль к поверхности
  float                   xyz_distance; // Расстояние до центра координат
  std::array<Vector3, 4>  vertices;     // Вершины прямоугольника стены

public:
  Wall(SurfaceType type, const Vector3& normal, float distance, 
       const std::array<Vector3, 4>& vertices);

  SurfaceType get_type() const;
  Vector3 get_normal() const;
  const std::array<Vector3, 4>& get_vertices() const; 
  Vector3 get_center() const;

  float calc_distance_to_point(const Vector3& point) const;
};

#endif
