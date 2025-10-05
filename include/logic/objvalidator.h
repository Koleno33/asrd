#ifndef OBJVALIDATOR_H
#define OBJVALIDATOR_H

#include <exception>
#include <pybind11/embed.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pylifecycle.h>
#include <vector>
#include <graphics/cube.h>
#include <iostream>
#include <filesystem>

namespace py = pybind11;

class __attribute__((visibility("default"))) CubeValidator
{
public:
  CubeValidator();
  Color validate_cubes(const std::vector<Cube*>& cubes);
private:
  py::scoped_interpreter guard {  };
};

#endif

