#ifndef OBJVALIDATOR_H
#define OBJVALIDATOR_H

#include <pybind11/embed.h>
#include <graphics/object.h>
#include <graphics/room.h>

namespace py = pybind11;

class OBJECT_API ObjValidator
{
public:
  ObjValidator();
  void validate(const std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Room> room);
private:
  Color get_color_from_status(const std::string& status);
  std::shared_ptr<Object> find_object_by_id(const std::vector<std::shared_ptr<Object>>& objects, uint64_t target_id);
  py::scoped_interpreter guard {  };
};

#endif

