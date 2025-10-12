#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <cmath>
#include <raymath.h>

Cube::Cube(const Vector3& pos, const Vector3& size, Color color)
  : Object(pos), size(size) 
{
  set_color(color);
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

float Cube::calculate_distance(const Object& other) const
{
  return other.calculate_distance_to_cube(*this);
}

float Cube::calculate_distance_to_cube(const Cube& other) const
{
  Vector3 diff = {
    fabs(position.x - other.position.x) - (size.x + other.size.x) * 0.5f,
    fabs(position.y - other.position.y) - (size.y + other.size.y) * 0.5f,
    fabs(position.z - other.position.z) - (size.z + other.size.z) * 0.5f
  };
  return Vector3Length({fmaxf(diff.x, 0), fmaxf(diff.y, 0), fmaxf(diff.z, 0)});
}

float Cube::calculate_distance_to_sphere(const Sphere& other) const
{
  Vector3 closest = {
    fmaxf(position.x - size.x * 0.5f, fminf(other.get_position().x, position.x + size.x * 0.5f)),
    fmaxf(position.y - size.y * 0.5f, fminf(other.get_position().y, position.y + size.y * 0.5f)),
    fmaxf(position.z - size.z * 0.5f, fminf(other.get_position().z, position.z + size.z * 0.5f))
  };
  return fmaxf(0, Vector3Distance(closest, other.get_position()) - other.get_radius());
}

bool Cube::check_collision(const Object& other) const
{   
  return other.check_collision_with_cube(*this);
}

bool Cube::check_collision_with_cube(const Cube& other) const {
  return (fabs(position.x - other.position.x) < (size.x + other.size.x) * 0.5f) &&
         (fabs(position.y - other.position.y) < (size.y + other.size.y) * 0.5f) &&
         (fabs(position.z - other.position.z) < (size.z + other.size.z) * 0.5f);
}

bool Cube::check_collision_with_sphere(const Sphere& other) const {
  Vector3 closest = {
    fmaxf(position.x - size.x * 0.5f, fminf(other.get_position().x, position.x + size.x * 0.5f)),
    fmaxf(position.y - size.y * 0.5f, fminf(other.get_position().y, position.y + size.y * 0.5f)),
    fmaxf(position.z - size.z * 0.5f, fminf(other.get_position().z, position.z + size.z * 0.5f))
  };
  return Vector3DistanceSqr(closest, other.get_position()) <= (other.get_radius() * other.get_radius());
}

