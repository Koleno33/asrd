#include <graphics/cube.h>
#include <cmath>
#include <vector>

Cube::Cube(const Vector3& pos, const Vector3& size, Color color)
  : Object(pos), size(size), color(color) 
{
}

const char* Cube::get_type() const 
{
  return "Cube";
}

void Cube::draw() const 
{
  // Using raylib for visualization
  DrawCubeV(position, size, color);
  DrawCubeWiresV(position, size, DARKGRAY);
}

void Cube::set_size(const Vector3& newsize) 
{
  size = newsize;
}


void Cube::set_color(Color newcolor) 
{
  color = newcolor;
}

float Cube::calculate_distance(const Cube& other) const
{
  float dx = position.x - other.position.x;
  float dy = position.y - other.position.y;
  float dz = position.z - other.position.z;

  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool Cube::check_collision(const Cube& other) const
{   
  Vector3 halfA = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
  Vector3 halfB = { other.size.x * 0.5f, other.size.y * 0.5f, other.size.z * 0.5f };

  bool overlapx = std::abs(position.x - other.position.x) <= (halfA.x + halfB.x);
  bool overlapy = std::abs(position.y - other.position.y) <= (halfA.y + halfB.y);
  bool overlapz = std::abs(position.z - other.position.z) <= (halfA.z + halfB.z);

  return overlapx && overlapy && overlapz;
}

