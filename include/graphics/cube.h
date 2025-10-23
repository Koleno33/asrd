#ifndef CUBE_H
#define CUBE_H

#include <graphics/object.h>

class OBJECT_API Cube : public Object {
private:
  Vector3 size {};

public:
  Cube(const Vector3& pos, const Vector3& size, Color color = WHITE);
  
  const char* get_type() const override;
  void        draw() const override;
  
  Vector3 get_size() const { return size; };
  void    set_size(const Vector3 &newsize);

  float   calculate_distance(const Object& other) const override;
  bool    check_collision(const Object& other) const override;

  float  calculate_distance_to_wall(const Wall& wall) const override;

  float  calculate_distance_to_cube(const Cube& other) const override;
  float  calculate_distance_to_sphere(const Sphere& other) const override;
  bool   check_collision_with_cube(const Cube& other) const override;
  bool   check_collision_with_sphere(const Sphere& other) const override;
};

#endif
