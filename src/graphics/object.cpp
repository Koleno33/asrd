#include <graphics/object.h>

// Инициализация статического счетчика ID
std::atomic<uint64_t> Object::nextid{1};

Object::Object(const Vector3& pos) 
  : id(nextid.fetch_add(1)), position(pos), angle_y(0.0f)
{
}

uint64_t Object::get_id() const 
{
  return id;
}

Vector3 Object::get_position() const 
{
  return position;
}

Color Object::get_color() const
{
  return color;
}

float Object::get_angle() const
{
  return angle_y;
}

void Object::set_position(const Vector3& newpos) 
{
  position = newpos;
}

void Object::set_color(Color newcolor) 
{
  color = newcolor;
}

void Object::set_angle(float new_angle)
{
  angle_y = new_angle;
}

bool Object::is_locked() const {
    return locked;
}

void Object::set_locked(bool locked_) {
    locked = locked_;
}
