#ifndef USEROBJECT_H
#define USEROBJECT_H

#include <graphics/object.h>
#include <string>

class OBJECT_API UserObject : public Object {
private:
    // ---------- поля для 3D-визуализации ----------
    Model  model;                   // модель raylib (загружается из .obj)
    bool   model_loaded = false;    // признак загруженной модели

    // ---------- геометрические параметры ----------
    Vector3      scale;                // масштаб по осям (может быть неравномерным)
    BoundingBox  local_bounds;         // локальный AABB модели (не масштабированный)
    Vector3      half_extents;         // половины габаритов ориентированного бокса (с учётом scale)
    Vector3      local_center_offset;  // смещение от (0,0,0) модели до центра её AABB

    // ---------- служебные методы ----------
    void update_half_extents();   // пересчёт half_extents из local_bounds * scale / 2

    // ---------- имена ----------
    std::string internal_name;    // имя для правил
    std::string display_name;     // имя для отображения пользователю

public:
    // Конструктор без модели – геометрия задаётся позже через load_from_file
    UserObject(const Vector3& pos, const Vector3& scale,
               const std::string& internalName = "",
               const std::string& displayName = "",
               Color color = WHITE);
    ~UserObject() override;

    // Запрет копирования (владеет ресурсом Model, но для clone – см. примечания)
    UserObject(const UserObject&) = delete;
    UserObject& operator=(const UserObject&) = delete;

    // ---------- переопределённые виртуальные методы Object ----------
    const char* get_type() const override;               // "UserObject"
    void        draw() const override;                   // рисует модель, если загружена

    // ---------- работа с моделью ----------
    bool load_from_file(const std::string& path);        // загрузка .obj, установка AABB
    void unload();                                       // выгрузка модели

    // ---------- доступ к параметрам ----------
    BoundingBox  get_bounds() const;                     // возвращает world-space AABB (или локальный)
    Vector3      get_scale() const;
    Vector3      get_half_extents() const;
    Vector3      get_local_center_offset() const;

    void         set_scale(const Vector3& new_scale);

    const std::string& get_internal_name() const;
    void set_internal_name(const std::string& name);

    const std::string& get_display_name() const;
    void set_display_name(const std::string& name);

    // ---------- геометрические запросы (все наследуются от Object) ----------
    float calculate_distance(const Object& other) const override;
    float calculate_distance_to_wall(const Wall& wall) const override;

    float calculate_distance_to_cube(const Cube& other) const override;
    float calculate_distance_to_sphere(const Sphere& other) const override;
    bool  check_collision_with_cube(const Cube& other) const override;
    bool  check_collision_with_sphere(const Sphere& other) const override;

    bool check_collision(const Object& other) const override;

    float get_projection_on_axis(const Vector3& axis) const override;

    float calculate_distance_to_userobject(const UserObject& other) const override;
    bool  check_collision_with_userobject(const UserObject& other) const override;

    // ---------- клонирование (для генетического алгоритма) ----------
    std::shared_ptr<Object> clone() const override;      // создаёт лёгкую копию без загрузки модели
};

#endif // USEROBJECT_H
