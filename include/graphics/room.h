#ifndef ROOM_H
#define ROOM_H

#include <raylib.h>
#include <array>
#include <vector>
#include <graphics/wall.h>

class Room 
{
private:
  Vector3 origin;
  Vector3 dimensions;
  std::array<Wall*, 6> walls;
  Color wireframe_color;
public:
  Room(const Vector3& origin, const Vector3& dimensions,
       const Color& wireframe_color = RED);
  ~Room();

  Vector3                get_center() const;
  std::vector<Vector3*>  get_wf_vertices() const;                       // Массив всех вершин помещения для каркаса
  Vector3                get_origin() const;
  Vector3                get_dimensions() const;
  Color                  get_wf_color() const;
  std::array<Wall*, 6>   get_walls() const;
  const Wall*            get_wall(SurfaceType type) const;

  void                   init_walls();
  float                  get_near_distance(const Vector3& point) const; // Расстояние от точки до ближайшей стены
  bool                   is_inside(const Vector3& point) const; 
  void                   draw(const Vector3& camera_pos) const;
};

#endif
