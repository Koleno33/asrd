#include <graphics/sphere.h>
#include <graphics/cube.h>
#include <graphics/wall.h>
#include <cmath>
#include <raymath.h>

Sphere::Sphere(const Vector3& pos, float radius, Color color)
  : Object(pos), radius(radius)
{
  set_color(color);
}

const char* Sphere::get_type() const 
{
  return "Sphere";
}

void Sphere::draw() const 
{
  // Using raylib for visualization
  DrawSphere(position, radius, color);
  DrawSphereWires(position, radius, DEFAULT_RINGS, DEFAULT_SLICES, DARKGRAY);
}

void Sphere::set_radius(float newradius) 
{
  radius = newradius;
}

float Sphere::calculate_distance_to_wall(const Wall& wall) const 
{
  // Вычисляем расстояние от центра сферы до стены
  float center_distance = wall.calc_distance_to_point(position);
  
  // Вычитаем радиус, чтобы получить расстояние до поверхности сферы
  float surface_distance = center_distance - radius;
  
  return surface_distance;
}

float Sphere::calculate_distance(const Object& other) const
{
  return other.calculate_distance_to_sphere(*this);
}

bool Sphere::check_collision(const Object& other) const
{   
  return other.check_collision_with_sphere(*this);
}

float Sphere::calculate_distance_to_cube(const Cube& other) const
{
  // delegate to Cube
  return other.calculate_distance_to_sphere(*this);
}

float Sphere::calculate_distance_to_sphere(const Sphere& other) const {
  return fmaxf(0, Vector3Distance(position, other.position) - radius - other.radius);
}

bool Sphere::check_collision_with_cube(const Cube& other) const {
  // delegate to Cube
  return other.check_collision_with_sphere(*this); 
}

bool Sphere::check_collision_with_sphere(const Sphere& other) const {
  return Vector3DistanceSqr(position, other.position) <= (radius + other.radius) * (radius + other.radius);
}

