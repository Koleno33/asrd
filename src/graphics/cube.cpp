#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <graphics/userobject.h>
#include <graphics/wall.h>
#include <cmath>
#include <raymath.h>
#include <rlgl.h>

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
  rlPushMatrix();
      rlTranslatef(position.x, position.y, position.z);
      rlRotatef(angle_y, 0.0f, 1.0f, 0.0f);

      // position is 0, 0, 0 after translation
      DrawCubeV({0, 0, 0}, size, color);
      DrawCubeWiresV({0, 0, 0}, size, DARKGRAY);
  rlPopMatrix();
}

void Cube::set_size(const Vector3& newsize) 
{
  size = newsize;
}

float Cube::calculate_distance_to_wall(const Wall& wall) const
{
  Vector3 wall_normal = wall.get_normal();
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_normal = Vector3RotateByAxisAngle(wall_normal, {0.0f, 1.0f, 0.0f}, -angle_rad);

  float half_proj = fabsf(local_normal.x) * size.x * 0.5f +
                    fabsf(local_normal.y) * size.y * 0.5f +
                    fabsf(local_normal.z) * size.z * 0.5f;

  float center_distance = wall.calc_distance_to_point(position);
  return center_distance - half_proj;
}

float Cube::calculate_distance(const Object& other) const
{
  return other.calculate_distance_to_cube(*this);
}

float Cube::calculate_distance_to_cube(const Cube& other) const
{
  float a1 = angle_y * DEG2RAD;
  float a2 = other.angle_y * DEG2RAD;

  Vector3 h1 = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
  Vector3 h2 = { other.size.x * 0.5f, other.size.y * 0.5f, other.size.z * 0.5f };

  // Vertical distance
  float dy = fabsf(position.y - other.position.y) - (h1.y + h2.y);
  float dist_y = fmaxf(0.0f, dy);

  // 2D distance in XZ plane (OBB vs OBB with parallel Y axis)
  Vector2 c1 = { position.x, position.z };
  Vector2 c2 = { other.position.x, other.position.z };
  Vector2 e1 = { h1.x, h1.z };
  Vector2 e2 = { h2.x, h2.z };

  float cos_a1 = cosf(a1), sin_a1 = sinf(a1);
  float cos_a2 = cosf(a2), sin_a2 = sinf(a2);

  auto compute_gap = [&](Vector2 axis) -> float {
      float d = (c2.x - c1.x) * axis.x + (c2.y - c1.y) * axis.y;
      float r1 = fabsf(e1.x * (cos_a1 * axis.x + sin_a1 * axis.y)) +
                 fabsf(e1.y * (-sin_a1 * axis.x + cos_a1 * axis.y));
      float r2 = fabsf(e2.x * (cos_a2 * axis.x + sin_a2 * axis.y)) +
                 fabsf(e2.y * (-sin_a2 * axis.x + cos_a2 * axis.y));
      return fabsf(d) - (r1 + r2);
  };

  float max_gap = 0.0f;
  float gap;

  gap = compute_gap({cos_a1, sin_a1});        // axis: local X of cube1
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a1, cos_a1});       // axis: local Z of cube1
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({cos_a2, sin_a2});        // axis: local X of cube2
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a2, cos_a2});       // axis: local Z of cube2
  if (gap > max_gap) max_gap = gap;

  float dist_xz = fmaxf(0.0f, max_gap);
  return sqrtf(dist_xz * dist_xz + dist_y * dist_y);
}

float Cube::calculate_distance_to_sphere(const Sphere& other) const
{
  Vector3 diff = Vector3Subtract(other.get_position(), position);
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_diff = Vector3RotateByAxisAngle(diff, {0.0f, 1.0f, 0.0f}, -angle_rad);
  Vector3 half = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };

  Vector3 closest = {
      fmaxf(-half.x, fminf(local_diff.x, half.x)),
      fmaxf(-half.y, fminf(local_diff.y, half.y)),
      fmaxf(-half.z, fminf(local_diff.z, half.z))
  };

  return fmaxf(0.0f, Vector3Distance(closest, local_diff) - other.get_radius());
}

bool Cube::check_collision(const Object& other) const
{   
  return other.check_collision_with_cube(*this);
}

bool Cube::check_collision_with_cube(const Cube& other) const 
{
  // Quick Y check
  float dy = fabsf(position.y - other.position.y);
  if (dy >= (size.y + other.size.y) * 0.5f) return false;

  float a1 = angle_y * DEG2RAD;
  float a2 = other.angle_y * DEG2RAD;

  Vector2 c1 = { position.x, position.z };
  Vector2 c2 = { other.position.x, other.position.z };
  Vector2 e1 = { size.x * 0.5f, size.z * 0.5f };
  Vector2 e2 = { other.size.x * 0.5f, other.size.z * 0.5f };

  float cos_a1 = cosf(a1), sin_a1 = sinf(a1);
  float cos_a2 = cosf(a2), sin_a2 = sinf(a2);

  auto overlap_on_axis = [&](Vector2 axis) -> bool {
      float d = fabsf((c2.x - c1.x) * axis.x + (c2.y - c1.y) * axis.y);
      float r1 = fabsf(e1.x * (cos_a1 * axis.x + sin_a1 * axis.y)) +
                 fabsf(e1.y * (-sin_a1 * axis.x + cos_a1 * axis.y));
      float r2 = fabsf(e2.x * (cos_a2 * axis.x + sin_a2 * axis.y)) +
                 fabsf(e2.y * (-sin_a2 * axis.x + cos_a2 * axis.y));
      return d <= (r1 + r2 + 1e-6f); // small tolerance
  };

  if (!overlap_on_axis({cos_a1, sin_a1})) return false;   // X axis of cube1
  if (!overlap_on_axis({-sin_a1, cos_a1})) return false;  // Z axis of cube1
  if (!overlap_on_axis({cos_a2, sin_a2})) return false;   // X axis of cube2
  if (!overlap_on_axis({-sin_a2, cos_a2})) return false;  // Z axis of cube2

  return true;
}

bool Cube::check_collision_with_sphere(const Sphere& other) const 
{
  Vector3 diff = Vector3Subtract(other.get_position(), position);
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_diff = Vector3RotateByAxisAngle(diff, {0.0f, 1.0f, 0.0f}, -angle_rad);
  Vector3 half = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };

  Vector3 closest = {
      fmaxf(-half.x, fminf(local_diff.x, half.x)),
      fmaxf(-half.y, fminf(local_diff.y, half.y)),
      fmaxf(-half.z, fminf(local_diff.z, half.z))
  };

  return Vector3DistanceSqr(closest, local_diff) <= (other.get_radius() * other.get_radius());
}

float Cube::get_projection_on_axis(const Vector3& axis) const
{
    // Поворачиваем ось обратно в локальную систему куба
    float angle_rad = angle_y * DEG2RAD;
    // Обратный поворот вокруг Y: поворачиваем ось на -angle_y
    Vector3 local_axis = Vector3RotateByAxisAngle(axis, {0.0f, 1.0f, 0.0f}, -angle_rad);
    // Проекция полуразмеров
    return fabsf(local_axis.x) * size.x * 0.5f +
           fabsf(local_axis.y) * size.y * 0.5f +
           fabsf(local_axis.z) * size.z * 0.5f;
}

std::shared_ptr<Object> Cube::clone() const 
{
    auto newCube = std::make_shared<Cube>(position, size, color);
    newCube->set_angle(angle_y);
    if (locked) newCube->set_locked(true);
    return newCube;
}

float Cube::calculate_distance_to_userobject(const UserObject& other) const 
{
    return other.calculate_distance_to_cube(*this);
}

bool Cube::check_collision_with_userobject(const UserObject& other) const 
{
    return other.check_collision_with_cube(*this);
}
