#ifndef OBJVALIDATOR_H
#define OBJVALIDATOR_H

#include <pybind11/embed.h>
#include <graphics/cube.h>

namespace py = pybind11;

class OBJECT_API CubeValidator
{
public:
  CubeValidator();
  Color validate_cubes(const std::vector<Cube*>& cubes);
private:
  py::scoped_interpreter guard {  };
};

#endif

