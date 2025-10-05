#ifndef CUBE_H
#define CUBE_H

#include <graphics/object.h>

class OBJECT_API Cube : public Object {
private:
  Vector3 size {};
  Color color {};

public:
  Cube(const Vector3& pos, const Vector3& size, Color color = WHITE);
  
  const char* get_type() const override;
  void        draw() const override;
  
  Vector3 get_size() const { return size; };
  void    set_size(const Vector3 &newsize);

  Color   get_color() const { return color; };
  void    set_color(Color newcolor);

  float   calculate_distance(const Cube& other) const;
  bool    check_collision(const Cube& other) const;
};

#endif
