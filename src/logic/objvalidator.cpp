#include <logic/objvalidator.h>
#include <pybind11/pytypes.h>
#include <iostream>

Color ObjValidator::get_color_from_status(const std::string& status)
{
  Color rescolor;
  if (status == "ok") rescolor = GRAY;
  else if (status == "violated") rescolor = ORANGE;
  else if (status == "invalid") rescolor = RED;
  else rescolor = GRAY;
  return rescolor;
}

Object* find_object_by_id(const std::vector<Object*>& objects, uint64_t target_id) {
  auto it = std::find_if(objects.begin(), objects.end(),
    [target_id](Object* obj) {
        return obj->get_id() == target_id;
    });
  
  return (it != objects.end()) ? *it : nullptr;
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

void ObjValidator::validate(const std::vector<Object*>& objs)
{
  try {
    py::module_ validator_module = py::module_::import("validator");

    py::list obj_list;
    for (Object* obj : objs) {
      obj_list.append(py::cast(obj));
    }

    py::list valresults = validator_module.attr("validate")(obj_list);

    Object* curobj;
    for (auto vresult : valresults) {
      std::string value = vresult["value"].cast<std::string>();
      py::dict attrs = vresult["attrs"];

      curobj = find_object_by_id(objs, attrs["obja"].cast<uint64_t>());
      curobj->set_color(get_color_from_status(value));
      curobj = find_object_by_id(objs, attrs["objb"].cast<uint64_t>());
      curobj->set_color(get_color_from_status(value));
    }
  }
  catch (const py::error_already_set& e) {
    std::cerr << "Python validation error: " << e.what() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Validation error: " << e.what() << '\n';
  }
}

