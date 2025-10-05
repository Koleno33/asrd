#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <graphics/cube.h>

namespace py = pybind11;

PYBIND11_MODULE(cubes_module, handle)
{
  handle.doc() = "Test module for cubes operations.";

  py::class_<Cube>(handle, "Cube")
    // constructors
    .def(py::init<>())

    // methods
    .def("calculate_distance", &Cube::calculate_distance)
    .def("check_collision", &Cube::check_collision)

    // getters
    .def_property_readonly("id", &Cube::getId)
    .def_property("position", &Cube::getPosition, &Cube::setPosition)
    .def_property("size", &Cube::getSize, &Cube::setSize);
 
    // representations
     py::bind_vector<std::vector<Cube*>>(handle, "VectorCubePtr");   
    ;

}

