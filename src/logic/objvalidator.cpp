#include <logic/objvalidator.h>
#include <iostream>
#include <pybind11/pytypes.h>

Color ObjValidator::get_color_from_status(const std::string& status)
{
  Color rescolor;
  if (status == "ok") rescolor = GRAY;
  else if (status == "violated") rescolor = ORANGE;
  else if (status == "invalid") rescolor = RED;
  else rescolor = GRAY;
  return rescolor;
}

ColorMap ObjValidator::group_objects_by_color(const std::vector<py::dict>& vresults)
{
  std::map<uint64_t, std::string> object_status;
  ColorMap color_groups;

  for (const auto& vresult : vresults) {
    std::string value = vresult["value"].cast<std::string>();
    py::dict attrs = vresult["attrs"];

    uint64_t obja = attrs["obja"].cast<uint64_t>();
    uint64_t objb = attrs["objb"].cast<uint64_t>();

    Color color = get_color_from_status(value);
    color_groups[color].push_back(obja);
    color_groups[color].push_back(objb);
  }
  
  return color_groups;
}

ObjValidator::ObjValidator() 
{
  try {
    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("append")(".");

    py::module_::import("objects_module");
  }
  catch (const std::exception& e) {
    std::cerr << "Python init error: " << e.what() << '\n';
  }
}

ColorMap ObjValidator::validate(const std::vector<Object*>& objs)
{
  ColorMap result;

  try {
    py::module_ validator_module = py::module_::import("validator");

    py::list obj_list;
    for (Object* obj : objs) {
      obj_list.append(py::cast(obj));
    }

    py::list valresults = validator_module.attr("validate")(obj_list);

    std::vector<py::dict> vresults_vec {};
    for (auto vresult : valresults) {
      vresults_vec.push_back(vresult.cast<py::dict>());
    }

    result = group_objects_by_color(vresults_vec);
  }
  catch (const py::error_already_set& e) {
    std::cerr << "Python validation error: " << e.what() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Validation error: " << e.what() << '\n';
  }

  return result;
}

