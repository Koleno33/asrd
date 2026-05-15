#include <graphics/room.h>
#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <graphics/userobject.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <cmath>
#include <iostream>
#include <typeinfo>
#include <raymath.h>

Room::Room(const Vector3& origin, const Vector3& dimensions, const Color& wireframe_color)
    : origin(origin), dimensions(dimensions), wireframe_color(wireframe_color) 
{
  init_walls();
}

void Room::init_walls() 
{
  float half_width = dimensions.x / 2.0f;
  float height = dimensions.y;
  float half_depth = dimensions.z / 2.0f;

  // Углы комнаты относительно центра пола
  float left = origin.x - half_width;
  float right = origin.x + half_width;
  float bottom = origin.y;
  float top = origin.y + height;
  float front = origin.z - half_depth;
  float back = origin.z + half_depth;

  // Пол и потолок
  walls[0] = std::make_unique<Wall>(SurfaceType::FLOOR, 
                     Vector3{0.0f, 1.0f, 0.0f}, bottom,
                     std::array<Vector3, 4>{
                         Vector3{left, bottom, front},
                         Vector3{right, bottom, front},
                         Vector3{right, bottom, back},
                         Vector3{left, bottom, back}
                     });

  walls[1] = std::make_unique<Wall>(SurfaceType::CEILING,
                     Vector3{0.0f, -1.0f, 0.0f}, -top,
                     std::array<Vector3, 4>{
                         Vector3{left, top, front},
                         Vector3{right, top, front},
                         Vector3{right, top, back},
                         Vector3{left, top, back}
                     });

  // Стены
  walls[2] = std::make_unique<Wall>(SurfaceType::WALL_FRONT,
                     Vector3{0.0f, 0.0f, 1.0f}, front,
                     std::array<Vector3, 4>{
                         Vector3{left, bottom, front},
                         Vector3{left, top, front},
                         Vector3{right, top, front},
                         Vector3{right, bottom, front}
                     });

  walls[3] = std::make_unique<Wall>(SurfaceType::WALL_BACK,
                     Vector3{0.0f, 0.0f, -1.0f}, -back,
                     std::array<Vector3, 4>{
                         Vector3{left, bottom, back},
                         Vector3{right, bottom, back},
                         Vector3{right, top, back},
                         Vector3{left, top, back}
                     });

  walls[4] = std::make_unique<Wall>(SurfaceType::WALL_LEFT,
                     Vector3{1.0f, 0.0f, 0.0f}, left,
                     std::array<Vector3, 4>{
                         Vector3{left, bottom, front},
                         Vector3{left, bottom, back},
                         Vector3{left, top, back},
                         Vector3{left, top, front}
                     });

  walls[5] = std::make_unique<Wall>(SurfaceType::WALL_RIGHT,
                     Vector3{-1.0f, 0.0f, 0.0f}, -right,
                     std::array<Vector3, 4>{
                         Vector3{right, bottom, front},
                         Vector3{right, top, front},
                         Vector3{right, top, back},
                         Vector3{right, bottom, back}
                     });
}

void Room::draw(const Vector3& camera_position) const 
{
  bool camera_inside = is_point_inside(camera_position);
  
  for (const auto& wall : walls) {
    auto vertices = wall->get_vertices();
    
    if (camera_inside) {
      // Рисуем только каркас
      DrawLine3D(vertices[0], vertices[1], wireframe_color);
      DrawLine3D(vertices[1], vertices[2], wireframe_color);
      DrawLine3D(vertices[2], vertices[3], wireframe_color);
      DrawLine3D(vertices[3], vertices[0], wireframe_color);
    } 
    else {
      // Рисуем прозрачные стены
      Color transparent_color = wireframe_color;
      transparent_color.a = 80;
      
      // Рисуем заполненные стены
      DrawTriangle3D(vertices[0], vertices[1], vertices[2], transparent_color);
      DrawTriangle3D(vertices[0], vertices[2], vertices[3], transparent_color);
      
      // И каркас поверх
      DrawLine3D(vertices[0], vertices[1], wireframe_color);
      DrawLine3D(vertices[1], vertices[2], wireframe_color);
      DrawLine3D(vertices[2], vertices[3], wireframe_color);
      DrawLine3D(vertices[3], vertices[0], wireframe_color);
    }
  }
}

Vector3 Room::get_center() const 
{
  // Центр комнаты находится в центре пола поднятом на половине высоты
  return Vector3{
    origin.x,
    origin.y + dimensions.y / 2,
    origin.z
  };
}

std::vector<Vector3> Room::get_wf_vertices() const 
{
  std::vector<Vector3> vertices;
  for (const auto& wall : walls) {
    auto wallverts = wall->get_vertices();
    for (const auto& vert : wallverts) {
      vertices.push_back(vert);
    }
  }
  return vertices;
}

Vector3 Room::get_origin() const { return origin; }
Vector3 Room::get_dimensions() const { return dimensions; }
Color Room::get_wf_color() const { return wireframe_color; }

std::array<std::shared_ptr<const Wall>, 6> Room::get_walls() const 
{
  return {
    std::shared_ptr<const Wall>(walls[0].get(), [](const Wall*){}),
    std::shared_ptr<const Wall>(walls[1].get(), [](const Wall*){}),
    std::shared_ptr<const Wall>(walls[2].get(), [](const Wall*){}),
    std::shared_ptr<const Wall>(walls[3].get(), [](const Wall*){}),
    std::shared_ptr<const Wall>(walls[4].get(), [](const Wall*){}),
    std::shared_ptr<const Wall>(walls[5].get(), [](const Wall*){})
  };
}

std::shared_ptr<const Wall> Room::get_wall(SurfaceType type) const 
{
  for (const auto& wall : walls) {
    if (wall->get_type() == type) {
        return std::shared_ptr<const Wall>(wall.get(), [](const Wall*){});
    }
  }
  return nullptr;
}

float Room::get_near_distance(const Vector3& point) const 
{
  float min_dist = std::numeric_limits<float>::max();
  for (const auto& wall : walls) {
    float dist = wall->calc_distance_to_point(point);
    if (dist < min_dist) min_dist = dist;
  }
  return min_dist;
}

bool Room::is_point_inside(const Vector3& point) const 
{
  float half_width = dimensions.x / 2.0f;
  float half_depth = dimensions.z / 2.0f;

  return point.x >= origin.x - half_width && point.x <= origin.x + half_width &&
         point.y >= origin.y && point.y <= origin.y + dimensions.y &&
         point.z >= origin.z - half_depth && point.z <= origin.z + half_depth;
}

bool Room::is_obj_inside(const Object& obj) const 
{
  float half_width = dimensions.x / 2.0f;
  float half_depth = dimensions.z / 2.0f;

  float min_x = origin.x - half_width;
  float max_x = origin.x + half_width;
  float min_y = origin.y;
  float max_y = origin.y + dimensions.y;
  float min_z = origin.z - half_depth;
  float max_z = origin.z + half_depth;

  // Сфера (AABB сферы)
  const Sphere* sphere = dynamic_cast<const Sphere*>(&obj);
  if (sphere) {
      float r = sphere->get_radius();
      Vector3 center = sphere->get_center();      // == position
      return (center.x - r) >= min_x && (center.x + r) <= max_x &&
             (center.y - r) >= min_y && (center.y + r) <= max_y &&
             (center.z - r) >= min_z && (center.z + r) <= max_z;
  }

  // Куб (OBB только с поворотом вокруг Y)
  const Cube* cube = dynamic_cast<const Cube*>(&obj);
  if (cube) {
    Vector3 center = cube->get_center();        // == position
    Vector3 half = Vector3Scale(cube->get_size(), 0.5f);
    float angle = cube->get_angle() * DEG2RAD;

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    // Полуоси в мировом пространстве
    Vector3 axisX = { half.x * cos_a, 0, half.x * sin_a };
    Vector3 axisY = { 0, half.y, 0 };
    Vector3 axisZ = { -half.z * sin_a, 0, half.z * cos_a };

    // Проверка всех 8 вершин OBB
    for (int i = 0; i < 8; ++i) {
      Vector3 corner = center;
      corner = Vector3Add(corner, Vector3Scale(axisX, (i & 1) ? 1.0f : -1.0f));
      corner = Vector3Add(corner, Vector3Scale(axisY, (i & 2) ? 1.0f : -1.0f));
      corner = Vector3Add(corner, Vector3Scale(axisZ, (i & 4) ? 1.0f : -1.0f));

      if (corner.x < min_x || corner.x > max_x ||
          corner.y < min_y || corner.y > max_y ||
          corner.z < min_z || corner.z > max_z) {
          return false;
      }
    }
    return true;
  }

  // UserObject (аналогичен кубу, но размеры из half_extents)
  const UserObject* userObj = dynamic_cast<const UserObject*>(&obj);
  if (userObj) {
    Vector3 center = userObj->get_center();     // position + local_center_offset
    Vector3 half = userObj->get_half_extents();
    float angle = userObj->get_angle() * DEG2RAD;

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    Vector3 axisX = { half.x * cos_a, 0, half.x * sin_a };
    Vector3 axisY = { 0, half.y, 0 };
    Vector3 axisZ = { -half.z * sin_a, 0, half.z * cos_a };

    for (int i = 0; i < 8; ++i) {
      Vector3 corner = center;
      corner = Vector3Add(corner, Vector3Scale(axisX, (i & 1) ? 1.0f : -1.0f));
      corner = Vector3Add(corner, Vector3Scale(axisY, (i & 2) ? 1.0f : -1.0f));
      corner = Vector3Add(corner, Vector3Scale(axisZ, (i & 4) ? 1.0f : -1.0f));

      if (corner.x < min_x || corner.x > max_x ||
          corner.y < min_y || corner.y > max_y ||
          corner.z < min_z || corner.z > max_z) {
          return false;
      }
    }
    return true;
  }

  // Неизвестный тип
  return false;
}

