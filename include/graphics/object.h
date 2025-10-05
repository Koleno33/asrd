#ifndef CUBE_H
#define CUBE_H

#include <raylib.h>
#include <vector>
#include <cmath>
#include <cstdint>

class Object 
{
private:
  uint64_t id;
  Vector3 position;

public:
  // Конструктор по умолчанию
  Object() : id(0), position({0,0,0}) {}

  // Методы доступа
  uint64_t getId() const { return id; }
  Vector3 getPosition() const { return position; }

  // Сеттеры
  void setId(uint64_t newId) { id = newId; }
  void setPosition(Vector3 newPos) { position = newPos; }
};

#endif
