#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <graphics/cube.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

PYBIND11_MODULE(cubes_module, handle)
{
  handle.doc() = "Test module for cubes operations.";

  // Raylib Color class
  py::class_<Color>(handle, "Color")
      .def(py::init<>())
      .def_readwrite("r", &Color::r)
      .def_readwrite("g", &Color::g)
      .def_readwrite("b", &Color::b)
      .def_readwrite("a", &Color::a);

  // Object class
  py::class_<Object>(handle, "Object")
    .def_property_readonly("id", &Object::get_id)
    .def_property("position", &Object::get_position, &Object::set_position);

  // Cube class
  py::class_<Cube, Object>(handle, "Cube")
    // constructors
    .def(py::init<const Vector3&, const Vector3&, Color>(),
         py::arg("pos"), py::arg("size"), py::arg("color") = WHITE)

    // methods
    .def("calculate_distance", &Cube::calculate_distance)
    .def("check_collision", &Cube::check_collision)

    // getters
    .def_property_readonly("id", &Cube::get_id)
    .def_property("position", &Cube::get_position, &Cube::set_position)
    .def_property("size", &Cube::get_size, &Cube::set_size);
 
  // representations
  py::bind_vector<std::vector<Cube*>>(handle, "VectorCubePtr");   
}

