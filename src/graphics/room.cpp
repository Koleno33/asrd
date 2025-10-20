#include <graphics/room.h>
#include <algorithm>
#include <limits>

Room::Room(const Vector3& origin, const Vector3& dimensions, const Color& wireframe_color)
    : origin(origin), dimensions(dimensions), wireframe_color(wireframe_color) 
{
  init_walls();
}

Room::~Room() 
{
  for (auto wall : walls) {
    delete wall;
  }
}

void Room::init_walls() 
{
  float x = origin.x, y = origin.y, z = origin.z;
  float w = dimensions.x, h = dimensions.y, d = dimensions.z;

  // Пол и потолок
  walls[0] = new Wall(SurfaceType::FLOOR, 
                     Vector3{0.0f, 1.0f, 0.0f}, y,
                     std::array<Vector3, 4>{
                         Vector3{x, y, z},
                         Vector3{x + w, y, z},
                         Vector3{x + w, y, z + d},
                         Vector3{x, y, z + d}
                     });

  walls[1] = new Wall(SurfaceType::CEILING,
                     Vector3{0.0f, -1.0f, 0.0f}, -(y + h),
                     std::array<Vector3, 4>{
                         Vector3{x, y + h, z},
                         Vector3{x + w, y + h, z},
                         Vector3{x + w, y + h, z + d},
                         Vector3{x, y + h, z + d}
                     });

  // Стены
  walls[2] = new Wall(SurfaceType::WALL_FRONT,
                     Vector3{0.0f, 0.0f, 1.0f}, z,
                     std::array<Vector3, 4>{
                         Vector3{x, y, z},
                         Vector3{x, y + h, z},
                         Vector3{x + w, y + h, z},
                         Vector3{x + w, y, z}
                     });

  walls[3] = new Wall(SurfaceType::WALL_BACK,
                     Vector3{0.0f, 0.0f, -1.0f}, -(z + d),
                     std::array<Vector3, 4>{
                         Vector3{x, y, z + d},
                         Vector3{x + w, y, z + d},
                         Vector3{x + w, y + h, z + d},
                         Vector3{x, y + h, z + d}
                     });

  walls[4] = new Wall(SurfaceType::WALL_LEFT,
                     Vector3{1.0f, 0.0f, 0.0f}, x,
                     std::array<Vector3, 4>{
                         Vector3{x, y, z},
                         Vector3{x, y, z + d},
                         Vector3{x, y + h, z + d},
                         Vector3{x, y + h, z}
                     });

  walls[5] = new Wall(SurfaceType::WALL_RIGHT,
                     Vector3{-1.0f, 0.0f, 0.0f}, -(x + w),
                     std::array<Vector3, 4>{
                         Vector3{x + w, y, z},
                         Vector3{x + w, y + h, z},
                         Vector3{x + w, y + h, z + d},
                         Vector3{x + w, y, z + d}
                     });
}

void Room::draw(const Vector3& camera_position) const 
{
  bool camera_inside = is_inside(camera_position);
  
  for (auto wall : walls) {
    auto vertices = wall->get_vertices();
    
    if (camera_inside) {
      // Рисуем только каркас
      DrawLine3D(vertices[0], vertices[1], wireframe_color);
      DrawLine3D(vertices[1], vertices[2], wireframe_color);
      DrawLine3D(vertices[2], vertices[3], wireframe_color);
      DrawLine3D(vertices[3], vertices[0], wireframe_color);
    } else {
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
  return Vector3{
    origin.x + dimensions.x / 2,
    origin.y + dimensions.y / 2,
    origin.z + dimensions.z / 2
  };
}

std::vector<Vector3*> Room::get_wf_vertices() const 
{
  std::vector<Vector3*> vertices;
  for (const auto& wall : walls) {
    auto wallverts = wall->get_vertices();
    for (const auto& vert : wallverts) {
      vertices.push_back(new Vector3(vert));
    }
  }
  return vertices;
}

Vector3 Room::get_origin() const { return origin; }
Vector3 Room::get_dimensions() const { return dimensions; }
Color Room::get_wf_color() const { return wireframe_color; }
std::array<Wall*, 6> Room::get_walls() const { return walls; }

const Wall* Room::get_wall(SurfaceType type) const 
{
  auto it = std::find_if(walls.begin(), walls.end(), 
                        [type](Wall* wall) { return wall->get_type() == type; });
  return it != walls.end() ? *it : nullptr;
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

bool Room::is_inside(const Vector3& point) const 
{
    return point.x >= origin.x && point.x <= origin.x + dimensions.x &&
           point.y >= origin.y && point.y <= origin.y + dimensions.y &&
           point.z >= origin.z && point.z <= origin.z + dimensions.z;
}
