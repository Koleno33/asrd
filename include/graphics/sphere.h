#ifndef SPHERE_H
#define SPHERE_H

#include <graphics/object.h>

class OBJECT_API Sphere : public Object {
private:
  inline static constexpr int DEFAULT_RINGS = 16;
  inline static constexpr int DEFAULT_SLICES = 16;

  float   radius {};

public:
  Sphere(const Vector3& pos, float radius, Color color = WHITE);
  
  const char* get_type() const override;
  void        draw() const override;
  
  float   get_radius() const { return radius; };
  void    set_radius(float newradius);

  float   calculate_distance(const Object& other) const override;
  bool    check_collision(const Object& other) const override;

  float  calculate_distance_to_wall(const Wall& wall) const override;

  float  calculate_distance_to_cube(const Cube& other) const override;
  float  calculate_distance_to_sphere(const Sphere& other) const override;
  bool   check_collision_with_cube(const Cube& other) const override;
  bool   check_collision_with_sphere(const Sphere& other) const override;
};

#endif
