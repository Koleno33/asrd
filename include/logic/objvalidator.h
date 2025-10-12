#ifndef OBJVALIDATOR_H
#define OBJVALIDATOR_H

#include <pybind11/embed.h>
#include <graphics/object.h>

namespace py = pybind11;

class OBJECT_API ObjValidator
{
public:
  ObjValidator();
  Color get_color_from_status(const std::string& status);
  void validate(const std::vector<Object*>& objs);
private:
  py::scoped_interpreter guard {  };
};

#endif

