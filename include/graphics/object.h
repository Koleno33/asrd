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

class OBJECT_API Object 
{
protected:
  static std::atomic<uint64_t> nextid;
  uint64_t id;
  Vector3 position;

  // Защищенный конструктор для использования в производных классах
  Object(const Vector3& pos);

public:
  virtual ~Object() = default;

  // Удаляем копирование и присваивание для сохранения уникальности ID
  Object(const Object&) = delete;
  Object& operator=(const Object&) = delete;

  // Методы доступа
  uint64_t get_id() const;
  Vector3 get_position() const;

  // Сеттеры
  void set_position(const Vector3& new_pos);

  // Абстрактный метод для отрисовки
  virtual void draw() const = 0;

  // Абстрактный метод для получения типа объекта
  virtual const char* get_type() const = 0;
};

#endif
