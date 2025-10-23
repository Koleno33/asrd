#ifndef OBJECT_H
#define OBJECT_H

#include <raylib.h>
#include <atomic>

#ifdef _WIN32
  #ifdef BUILDING_DLL
    #define OBJECT_API __declspec(dllexport)
  #else
    #define OBJECT_API __declspec(dllimport)
  #endif
#else
  #define OBJECT_API __attribute__((visibility("default")))
#endif

class Cube;
class Sphere;
class Wall;

class OBJECT_API Object 
{
protected:
  static std::atomic<uint64_t> nextid;
  uint64_t id;
  Vector3 position;
  Color color;

  // Защищенный конструктор для использования в производных классах
  Object(const Vector3& pos);

public:
  virtual ~Object() = default;

  // Удаляем копирование и присваивание для сохранения уникальности ID
  Object(const Object&) = delete;
  Object& operator=(const Object&) = delete;

  // Методы доступа
  uint64_t get_id() const;
  Vector3  get_position() const;
  Color    get_color() const;

  // Сеттеры
  void set_position(const Vector3& new_pos);
  void set_color(Color);

  // Абстрактный метод для отрисовки
  virtual void draw() const = 0;

  // Абстрактный метод для получения типа объекта
  virtual const char* get_type() const = 0;

  // Виртуальные методы для коллизий
  virtual float calculate_distance(const Object& other) const = 0;
  virtual float calculate_distance_to_wall(const Wall& wall) const = 0;
  virtual bool check_collision(const Object& other) const = 0;

  // Перегруженные методы для конкретных типов
  virtual float calculate_distance_to_cube(const Cube& other) const = 0;
  virtual float calculate_distance_to_sphere(const Sphere& other) const = 0;
  virtual bool check_collision_with_cube(const Cube& other) const = 0;
  virtual bool check_collision_with_sphere(const Sphere& other) const = 0;
};

#endif
