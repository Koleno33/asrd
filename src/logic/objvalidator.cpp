#include <cstdint>
#include <logic/objvalidator.h>
#include <memory>
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

std::shared_ptr<Object> ObjValidator::find_object_by_id(const std::vector<std::shared_ptr<Object>>& objects, uint64_t target_id) {
  auto it = std::find_if(objects.begin(), objects.end(),
    [target_id](const std::shared_ptr<Object>& obj) {
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

void ObjValidator::validate(const std::vector<std::shared_ptr<Object>>& objs, std::shared_ptr<Room> room)
{
  try {
    py::module_ validator_module = py::module_::import("validator");

    py::list obj_list;
    for (std::shared_ptr<Object> obj : objs) {
      // py::object py_obj = py::cast(obj, py::return_value_policy::reference);
      obj_list.append(py::cast(obj));
    }

    py::list valresults = validator_module.attr("validate")(obj_list, room);

    std::cout << "=== VALIDATION RESULTS ===" << std::endl;
    std::cout << "Number of results: " << len(valresults) << std::endl;

    std::shared_ptr<Object> curobj;
    for (auto vresult : valresults) {
      std::string rule = vresult["attrs"]["type"].cast<std::string>();
      std::string value = vresult["value"].cast<std::string>();

      std::cout << "Rule: " << rule << ", Status: " << value << std::endl;

      if (rule == "min_distance") {
        py::dict attrs = vresult["attrs"];
        uint64_t obja_id = attrs["obja"].cast<uint64_t>();
        uint64_t objb_id = attrs["objb"].cast<uint64_t>();

        std::cout << "  Objects: " << obja_id << " and " << objb_id << std::endl;

        curobj = find_object_by_id(objs, obja_id);
        curobj->set_color(get_color_from_status(value));
        curobj = find_object_by_id(objs, objb_id);
        curobj->set_color(get_color_from_status(value));
      }
      else if (rule == "distance") {
        py::dict attrs = vresult["attrs"];
        uint64_t obj_id = attrs["obja"].cast<uint64_t>();

        curobj = find_object_by_id(objs, obj_id);
        curobj->set_color(get_color_from_status(value));
      }
      else {
        std::cout << "  Object not found" << std::endl;
      }
    }
    std::cout << "=== END VALIDATION RESULTS ===" << std::endl;
  }
  catch (const py::error_already_set& e) {
    std::cerr << "Python validation error: " << e.what() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Validation error: " << e.what() << '\n';
  }
}

