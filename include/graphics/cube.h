#ifndef CUBE_H
#define CUBE_H

#include <raylib.h>
#include <vector>
#include <cmath>
#include <cstdint>

class Cube 
{
private:
  uint64_t id;
  Vector3 position;
  Vector3 size;

public:
  // Конструктор по умолчанию
  Cube() : id(0), position({0,0,0}), size({1,1,1}) {}

  // Методы доступа
  uint64_t getId() const { return id; }
  Vector3 getPosition() const { return position; }
  Vector3 getSize() const { return size; }

  // Сеттеры
  void setId(uint64_t newId) { id = newId; }
  void setPosition(Vector3 newPos) { position = newPos; }
  void setSize(Vector3 newSize) { size = newSize; }

  double calculate_distance(const Cube& other) const;
  bool   check_collision(const Cube& other) const;
};

#endif
