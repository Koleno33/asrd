#include <graphics/userobject.h>
#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <graphics/wall.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <raymath.h>
#include <rlgl.h>

UserObject::UserObject(const Vector3& pos, const Vector3& scale,
                     const std::string& internalName,
                     const std::string& displayName,
                     Color color)
  : Object(pos),
    internal_name(internalName),
    display_name(displayName),
    model{ 0 },
    model_loaded(false),
    scale(scale),
    local_bounds{ {0,0,0}, {0,0,0} },
    half_extents{0,0,0},
    local_center_offset{0,0,0}
{
  set_color(color);
}

UserObject::~UserObject() 
{
  unload();
}

const char* UserObject::get_type() const 
{
  return "UserObject";
}

void UserObject::draw() const 
{
  if (model_loaded && model.meshCount > 0) {
      // Рисуем модель с позицией, поворотом вокруг Y и масштабом.
      // Цвет из свойства color выступает как оттеночный (tint).
      // Если нужно сохранить исходные цвета модели, tint можно передавать WHITE,
      // но здесь используется заданный пользователем цвет.
      DrawModelEx(model,
                  position,
                  { 0.0f, 1.0f, 0.0f },   // ось вращения
                  angle_y,
                  scale,
                  color);
  }
}

void UserObject::update_half_extents() 
{
  // Габариты локального AABB умножаются на масштаб и делятся пополам
  Vector3 size = Vector3Subtract(local_bounds.max, local_bounds.min);
  half_extents = {
      size.x * 0.5f * scale.x,
      size.y * 0.5f * scale.y,
      size.z * 0.5f * scale.z
  };
}

bool UserObject::load_from_file(const std::string& path) 
{
  std::cout << "  [trace] Loading model..." << std::endl;
  Model m = LoadModel(path.c_str());
  std::cout << "  [trace] LoadModel done. meshCount = " << m.meshCount << std::endl;

  if (m.meshCount == 0) {
    return false;
  }

  // Проверка валидности мешей
  for (int i = 0; i < m.meshCount; i++) {
    if (m.meshes[i].vertexCount == 0) {
      std::cerr << "  [error] Empty mesh at index " << i << std::endl;
      UnloadModel(m);
      return false;
    }
  }

  if (model_loaded) {
      UnloadModel(model);
  }

  model = m;
  model_loaded = true;

  std::cout << "  [trace] Getting AABB..." << std::endl;
  local_bounds = GetModelBoundingBox(model);
  std::cout << "  [trace] Updating half extents..." << std::endl;
  update_half_extents();
  return true;
}

void UserObject::unload() {
  if (!model_loaded) return;

  // Освобождаем только CPU-память (вершины, текстурные координаты и т.д.)
  // VAO/VBO не трогаем, чтобы избежать segfault из-за невалидных идентификаторов.
  for (int i = 0; i < model.meshCount; i++) {
    Mesh* mesh = &model.meshes[i];
    if (mesh->vertices != nullptr)  { RL_FREE(mesh->vertices);  mesh->vertices  = nullptr; }
    if (mesh->texcoords != nullptr) { RL_FREE(mesh->texcoords); mesh->texcoords = nullptr; }
    if (mesh->normals != nullptr)   { RL_FREE(mesh->normals);   mesh->normals   = nullptr; }
    if (mesh->colors != nullptr)    { RL_FREE(mesh->colors);    mesh->colors    = nullptr; }
    if (mesh->indices != nullptr)   { RL_FREE(mesh->indices);   mesh->indices   = nullptr; }
    // Если есть другие динамические поля (tangents, texcoords2), их тоже RL_FREE.
  }

  // Материалы и текстуры – оставляем как есть, они освободятся при закрытии программы.

  // Обнуляем модель
  model = { 0 };
  model_loaded = false;
  half_extents = { 0, 0, 0 };
}

void UserObject::set_scale(const Vector3& new_scale) 
{
  scale = new_scale;
  if (model_loaded) {
      update_half_extents();
  }
}

Vector3 UserObject::get_scale() const 
{
  return scale;
}

Vector3 UserObject::get_half_extents() const 
{ 
  return half_extents; 
}


Vector3 UserObject::get_local_center_offset() const
{
  return local_center_offset;
}

const std::string& UserObject::get_internal_name() const 
{
  return internal_name;
}

void UserObject::set_internal_name(const std::string& name) 
{
  internal_name = name;
}

const std::string& UserObject::get_display_name() const 
{
  return display_name;
}

void UserObject::set_display_name(const std::string& name) 
{
  display_name = name;
}

BoundingBox UserObject::get_bounds() const 
{
  // Возвращаем мировой AABB, который охватывает OBB.
  // Вычисляем 8 углов OBB и берём min/max по каждой координате.
  Vector3 corners[8];
  float hx = half_extents.x;
  float hy = half_extents.y;
  float hz = half_extents.z;

  // Локальные углы (до поворота)
  Vector3 local_corners[8] = {
    {-hx, -hy, -hz}, {-hx, -hy,  hz},
    {-hx,  hy, -hz}, {-hx,  hy,  hz},
    { hx, -hy, -hz}, { hx, -hy,  hz},
    { hx,  hy, -hz}, { hx,  hy,  hz}
  };

  // Поворот только вокруг Y
  float rad = angle_y * DEG2RAD;
  Vector3 world_min = { FLT_MAX, FLT_MAX, FLT_MAX };
  Vector3 world_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

  for (int i = 0; i < 8; i++) {
    Vector3 rot = Vector3RotateByAxisAngle(local_corners[i], {0,1,0}, rad);
    Vector3 world = Vector3Add(position, rot);
    world_min = Vector3Min(world_min, world);
    world_max = Vector3Max(world_max, world);
  }

  return { world_min, world_max };
}

// ---------- Distance calculations ----------

float UserObject::calculate_distance_to_wall(const Wall& wall) const 
{
  Vector3 wall_normal = wall.get_normal();
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_normal = Vector3RotateByAxisAngle(wall_normal, {0.0f, 1.0f, 0.0f}, -angle_rad);

  float half_proj = fabsf(local_normal.x) * half_extents.x +
                    fabsf(local_normal.y) * half_extents.y +
                    fabsf(local_normal.z) * half_extents.z;

  float center_distance = wall.calc_distance_to_point(position);
  return center_distance - half_proj;
}

float UserObject::calculate_distance_to_cube(const Cube& other) const 
{
  float a1 = angle_y * DEG2RAD;
  float a2 = other.get_angle() * DEG2RAD;

  Vector3 h1 = half_extents;  // already half
  Vector3 h2 = { other.get_size().x * 0.5f,
                 other.get_size().y * 0.5f,
                 other.get_size().z * 0.5f };

  // Vertical distance
  float dy = fabsf(position.y - other.get_position().y) - (h1.y + h2.y);
  float dist_y = fmaxf(0.0f, dy);

  // 2D distance in XZ plane (OBB vs OBB)
  Vector2 c1 = { position.x, position.z };
  Vector2 c2 = { other.get_position().x, other.get_position().z };
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

  float gap = compute_gap({cos_a1, sin_a1});        // local X of this
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a1, cos_a1});             // local Z of this
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({cos_a2, sin_a2});              // local X of cube
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a2, cos_a2});             // local Z of cube
  if (gap > max_gap) max_gap = gap;

  float dist_xz = fmaxf(0.0f, max_gap);
  return sqrtf(dist_xz * dist_xz + dist_y * dist_y);
}

float UserObject::calculate_distance_to_sphere(const Sphere& other) const 
{
  Vector3 diff = Vector3Subtract(other.get_position(), position);
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_diff = Vector3RotateByAxisAngle(diff, {0.0f, 1.0f, 0.0f}, -angle_rad);

  Vector3 closest = {
      fmaxf(-half_extents.x, fminf(local_diff.x, half_extents.x)),
      fmaxf(-half_extents.y, fminf(local_diff.y, half_extents.y)),
      fmaxf(-half_extents.z, fminf(local_diff.z, half_extents.z))
  };

  return fmaxf(0.0f, Vector3Distance(closest, local_diff) - other.get_radius());
}

// Для объектов того же типа – двойная диспетчеризация
float UserObject::calculate_distance(const Object& other) const 
{
  // Предполагается, что в Object объявлен виртуальный метод
  // calculate_distance_to_userobject, и он реализован в Cube, Sphere и UserObject.
  return other.calculate_distance_to_userobject(*this);
}

float UserObject::calculate_distance_to_userobject(const UserObject& other) const 
{
  // Аналогично Cube-Cube, но используем half_extents обоих объектов
  float a1 = angle_y * DEG2RAD;
  float a2 = other.angle_y * DEG2RAD;

  Vector3 h1 = half_extents;
  Vector3 h2 = other.half_extents;

  float dy = fabsf(position.y - other.position.y) - (h1.y + h2.y);
  float dist_y = fmaxf(0.0f, dy);

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

  float gap = compute_gap({cos_a1, sin_a1});
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a1, cos_a1});
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({cos_a2, sin_a2});
  if (gap > max_gap) max_gap = gap;
  gap = compute_gap({-sin_a2, cos_a2});
  if (gap > max_gap) max_gap = gap;

  float dist_xz = fmaxf(0.0f, max_gap);
  return sqrtf(dist_xz * dist_xz + dist_y * dist_y);
}

// ---------- Collision checks ----------

bool UserObject::check_collision(const Object& other) const 
{
  return other.check_collision_with_userobject(*this);
}

bool UserObject::check_collision_with_cube(const Cube& other) const 
{
  float dy = fabsf(position.y - other.get_position().y);
  if (dy >= (half_extents.y + other.get_size().y * 0.5f))
      return false;

  float a1 = angle_y * DEG2RAD;
  float a2 = other.get_angle() * DEG2RAD;

  Vector2 c1 = { position.x, position.z };
  Vector2 c2 = { other.get_position().x, other.get_position().z };
  Vector2 e1 = { half_extents.x, half_extents.z };
  Vector2 e2 = { other.get_size().x * 0.5f, other.get_size().z * 0.5f };

  float cos_a1 = cosf(a1), sin_a1 = sinf(a1);
  float cos_a2 = cosf(a2), sin_a2 = sinf(a2);

  auto overlap_on_axis = [&](Vector2 axis) -> bool {
    float d = fabsf((c2.x - c1.x) * axis.x + (c2.y - c1.y) * axis.y);
    float r1 = fabsf(e1.x * (cos_a1 * axis.x + sin_a1 * axis.y)) +
               fabsf(e1.y * (-sin_a1 * axis.x + cos_a1 * axis.y));
    float r2 = fabsf(e2.x * (cos_a2 * axis.x + sin_a2 * axis.y)) +
               fabsf(e2.y * (-sin_a2 * axis.x + cos_a2 * axis.y));
    return d <= (r1 + r2 + 1e-6f);
  };

  if (!overlap_on_axis({cos_a1, sin_a1})) return false;
  if (!overlap_on_axis({-sin_a1, cos_a1})) return false;
  if (!overlap_on_axis({cos_a2, sin_a2})) return false;
  if (!overlap_on_axis({-sin_a2, cos_a2})) return false;

  return true;
}

bool UserObject::check_collision_with_sphere(const Sphere& other) const 
{
  Vector3 diff = Vector3Subtract(other.get_position(), position);
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_diff = Vector3RotateByAxisAngle(diff, {0.0f, 1.0f, 0.0f}, -angle_rad);

  Vector3 closest = {
      fmaxf(-half_extents.x, fminf(local_diff.x, half_extents.x)),
      fmaxf(-half_extents.y, fminf(local_diff.y, half_extents.y)),
      fmaxf(-half_extents.z, fminf(local_diff.z, half_extents.z))
  };

  return Vector3DistanceSqr(closest, local_diff) <= (other.get_radius() * other.get_radius());
}

bool UserObject::check_collision_with_userobject(const UserObject& other) const 
{
  // Аналогично check_collision_with_cube, но с half_extents обоих
  float dy = fabsf(position.y - other.position.y);
  if (dy >= (half_extents.y + other.half_extents.y))
      return false;

  float a1 = angle_y * DEG2RAD;
  float a2 = other.angle_y * DEG2RAD;

  Vector2 c1 = { position.x, position.z };
  Vector2 c2 = { other.position.x, other.position.z };
  Vector2 e1 = { half_extents.x, half_extents.z };
  Vector2 e2 = { other.half_extents.x, other.half_extents.z };

  float cos_a1 = cosf(a1), sin_a1 = sinf(a1);
  float cos_a2 = cosf(a2), sin_a2 = sinf(a2);

  auto overlap_on_axis = [&](Vector2 axis) -> bool {
      float d = fabsf((c2.x - c1.x) * axis.x + (c2.y - c1.y) * axis.y);
      float r1 = fabsf(e1.x * (cos_a1 * axis.x + sin_a1 * axis.y)) +
                 fabsf(e1.y * (-sin_a1 * axis.x + cos_a1 * axis.y));
      float r2 = fabsf(e2.x * (cos_a2 * axis.x + sin_a2 * axis.y)) +
                 fabsf(e2.y * (-sin_a2 * axis.x + cos_a2 * axis.y));
      return d <= (r1 + r2 + 1e-6f);
  };

  if (!overlap_on_axis({cos_a1, sin_a1})) return false;
  if (!overlap_on_axis({-sin_a1, cos_a1})) return false;
  if (!overlap_on_axis({cos_a2, sin_a2})) return false;
  if (!overlap_on_axis({-sin_a2, cos_a2})) return false;

  return true;
}

// ---------- Projection on arbitrary axis ----------

float UserObject::get_projection_on_axis(const Vector3& axis) const 
{
  float angle_rad = angle_y * DEG2RAD;
  Vector3 local_axis = Vector3RotateByAxisAngle(axis, {0.0f, 1.0f, 0.0f}, -angle_rad);
  return fabsf(local_axis.x) * half_extents.x +
         fabsf(local_axis.y) * half_extents.y +
         fabsf(local_axis.z) * half_extents.z;
}

// ---------- Clone ----------

std::shared_ptr<Object> UserObject::clone() const 
{
  // Создаём копию без загруженной модели – клон предназначен только для вычислений.
  auto newObj = std::make_shared<UserObject>(position, scale,
                                             internal_name, display_name,
                                             color);
  newObj->set_angle(angle_y);
  newObj->half_extents = half_extents;   // важно: чтобы геометрия совпадала
  newObj->local_bounds = local_bounds;   // тоже для полноты
  if (locked) newObj->set_locked(true);
  return newObj;
}
