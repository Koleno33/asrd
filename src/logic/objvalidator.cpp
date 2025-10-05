#include <logic/objvalidator.h>

CubeValidator::CubeValidator() 
{
  try {
    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("append")(".");

    py::module_::import("cubes_module");
  }
  catch (const std::exception& e) {
    std::cerr << "Python init error: " << e.what() << '\n';
  }
}

Color CubeValidator::validate_cubes(const std::vector<Cube*>& cubes)
{
  try {
    py::module_ validator_module = py::module_::import("validator");

    py::list cube_list;
    for (Cube* cube : cubes) {
        cube_list.append(py::cast(cube));
    }

    py::dict valres = validator_module.attr("validate")(cube_list);

    std::string status = valres["value"].cast<std::string>();

    if (status == "ok") return GREEN;
    else if (status == "violated") return ORANGE;
    else if (status == "invalid") return RED;

    return GRAY;
  }
  catch (const py::error_already_set& e) {
    std::cerr << "Python validation error: " << e.what() << '\n';
    return GRAY;
  }
  catch (const std::exception& e) {
    std::cerr << "Validation error: " << e.what() << '\n';
    return GRAY;
  }
}

