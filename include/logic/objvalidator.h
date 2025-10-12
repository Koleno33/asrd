#ifndef OBJVALIDATOR_H
#define OBJVALIDATOR_H

#include <pybind11/embed.h>
#include <graphics/cube.h>
#include <map>
#include <algorithm>
#include <cstdint>

namespace py = pybind11;

// for using Color into map 
struct ColorCompare
{
  bool operator()(const Color& lhs, const Color& rhs) const {
    return std::tie(lhs.r, lhs.g, lhs.b, lhs.a) < 
           std::tie(rhs.r, rhs.g, rhs.b, rhs.a);
  }
};
using ColorMap = std::map<Color, std::vector<uint64_t>, ColorCompare>;

class OBJECT_API ObjValidator
{
public:
  ObjValidator();
  Color get_color_from_status(const std::string& status);
  ColorMap validate(const std::vector<Object*>& objs);
  ColorMap group_objects_by_color(const std::vector<py::dict>& vresults);
private:
  py::scoped_interpreter guard {  };
};

#endif

