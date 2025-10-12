#include <graphics/object.h>

// Инициализация статического счетчика ID
std::atomic<uint64_t> Object::nextid{1};

Object::Object(const Vector3& pos) 
  : id(nextid.fetch_add(1)), position(pos) 
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

void Object::set_position(const Vector3& newpos) 
{
  position = newpos;
}

void Object::set_color(Color newcolor) 
{
  color = newcolor;
}

