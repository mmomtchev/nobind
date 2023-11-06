#include <fixtures/basic_class.h>
#include <vector>

std::vector<Hello *> get_ptr_array(unsigned number) {
  std::vector<Hello *> r;
  r.reserve(number);
  for (size_t i = 0; i < number; i++)
    r.push_back(new Hello("element " + std::to_string(number)));
  return r;
}

std::vector<std::string> put_ptr_array(const std::string &title, const std::vector<Hello *> &array) {
  std::vector<std::string> r;
  r.reserve(array.size());
  for (auto obj : array) {
    r.push_back(obj->Greet(title));
  }
  return r;
}

std::vector<Hello> get_obj_array(unsigned number) {
  std::vector<Hello> r;
  r.reserve(number);
  for (size_t i = 0; i < number; i++)
    r.emplace_back(Hello("element " + std::to_string(number)));
  return r;
}

std::vector<std::string> put_obj_array(const std::string &title, const std::vector<Hello> &array) {
  std::vector<std::string> r;
  r.reserve(array.size());
  for (auto obj : array) {
    r.push_back(obj.Greet(title));
  }
  return r;
}

#include <nobind.h>

NOBIND_MODULE(array, m) {
  m.def<Hello>("Hello")
    .cons<std::string &>()
    .def<&Hello::Id>("get_id")
    .def<&Hello::id, Nobind::readOnly>("id");
  m.def<&get_ptr_array>("get_ptr_array");
  m.def<&put_ptr_array>("put_ptr_array");
  m.def<&get_obj_array>("get_obj_array");
  m.def<&put_obj_array>("put_obj_array");
}
