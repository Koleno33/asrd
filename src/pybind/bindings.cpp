#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

PYBIND11_MODULE(objects_module, handle)
{
  handle.doc() = "Module for 3D objects operations.";

  // Raylib Vector3 class
  py::class_<Vector3>(handle, "Vector3")
    .def(py::init<>())
    .def(py::init<float, float, float>(),
         py::arg("x"), py::arg("y"), py::arg("z"))
    .def_readwrite("x", &Vector3::x)
    .def_readwrite("y", &Vector3::y)
    .def_readwrite("z", &Vector3::z)
    .def("__repr__", [](const Vector3& v) {
        return "Vector3(" + std::to_string(v.x) + ", " + 
                       std::to_string(v.y) + ", " + 
                       std::to_string(v.z) + ")";
    });

  // Raylib Color class
  py::class_<Color>(handle, "Color")
    .def(py::init<>())
    .def(py::init<unsigned char, unsigned char, unsigned char, unsigned char>(),
         py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255)
    .def_readwrite("r", &Color::r)
    .def_readwrite("g", &Color::g)
    .def_readwrite("b", &Color::b)
    .def_readwrite("a", &Color::a)
    .def("__repr__", [](const Color& c) {
        return "Color(" + std::to_string(c.r) + ", " + 
                     std::to_string(c.g) + ", " + 
                     std::to_string(c.b) + ", " + 
                     std::to_string(c.a) + ")";
    });

  // Color constants
  handle.attr("GRAY") = GRAY;
  handle.attr("RED") = RED;
  handle.attr("GREEN") = GREEN;
  handle.attr("ORANGE") = ORANGE;

  // Object class
  py::class_<Object, std::shared_ptr<Object>>(handle, "Object")
    .def_property_readonly("id", &Object::get_id)
    .def_property("position", &Object::get_position, &Object::set_position)
    .def("get_type", &Object::get_type)
    .def("calculate_distance", &Object::calculate_distance)
    .def("check_collision", &Object::check_collision);

  // Cube class
  py::class_<Cube, Object, std::shared_ptr<Cube>>(handle, "Cube")
    .def(py::init<const Vector3&, const Vector3&, Color>(),
       py::arg("pos"), py::arg("size"), py::arg("color") = WHITE,
       "Create a cube with position, size and color")
    
    // Properties
    .def_property("size", &Cube::get_size, &Cube::set_size)
    
    // Methods
    .def("calculate_distance", &Cube::calculate_distance)
    .def("check_collision", &Cube::check_collision)
    
    // String representation
    .def("__repr__", [](const Cube& c) {
      auto pos = c.get_position();
      auto size = c.get_size();
      return "Cube(pos=(" + std::to_string(pos.x) + ", " + 
                          std::to_string(pos.y) + ", " + 
                          std::to_string(pos.z) + 
                 "), size=(" + std::to_string(size.x) + ", " + 
                            std::to_string(size.y) + ", " + 
                            std::to_string(size.z) + "))";
  });

  // Sphere class
  py::class_<Sphere, Object, std::shared_ptr<Sphere>>(handle, "Sphere")
    .def(py::init<const Vector3&, float, Color>(),
         py::arg("pos"), py::arg("radius"), py::arg("color") = WHITE,
         "Create a sphere with position, radius and color")
    
    // Properties
    .def_property("radius", &Sphere::get_radius, &Sphere::set_radius)
    
    // Specific methods
    .def("calculate_distance", &Sphere::calculate_distance)
    .def("check_collision", &Sphere::check_collision)
    
    // String representation
    .def("__repr__", [](const Sphere& s) {
        auto pos = s.get_position();
        return "Sphere(pos=(" + std::to_string(pos.x) + ", " + 
                              std::to_string(pos.y) + ", " + 
                              std::to_string(pos.z) + 
                     "), radius=" + std::to_string(s.get_radius()) + ")";
  });

  // Vectors representations
  py::bind_vector<std::vector<std::shared_ptr<Object>>>(handle, "VectorObjectPtr");
  py::bind_vector<std::vector<std::shared_ptr<Cube>>>(handle, "VectorCubePtr");
  py::bind_vector<std::vector<std::shared_ptr<Sphere>>>(handle, "VectorSpherePtr");

  // Utility functions
  handle.def("calculate_distance_between", 
             [](const Object& obj1, const Object& obj2) {
                 return obj1.calculate_distance(obj2);
             },
             "Calculate distance between any two objects");

  handle.def("check_collision_between", 
             [](const Object& obj1, const Object& obj2) {
                 return obj1.check_collision(obj2);
             },
             "Check collision between any two objects");
}

