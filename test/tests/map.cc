#include <fixtures/basic_class.h>
#include <fixtures/critical.h>
#include <map>

std::map<std::string, Hello *> get_ptr_map(unsigned number) {
  std::map<std::string, Hello *> r;
  for (size_t i = 0; i < number; i++) {
    std::string name = "element " + std::to_string(i);
    r.insert({name, new Hello(name)});
  }
  return r;
}

std::map<std::string, std::string> put_ptr_map(const std::string &title, const std::map<std::string, Hello *> &map) {
  std::map<std::string, std::string> r;
  for (auto obj : map) {
    r.insert({obj.first, obj.second->Greet(title)});
  }
  return r;
}

std::map<std::string, Hello> get_obj_map(unsigned number) {
  std::map<std::string, Hello> r;
  for (size_t i = 0; i < number; i++) {
    std::string name = "element " + std::to_string(i);
    r.insert({name, Hello(name)});
  }
  return r;
}

std::map<std::string, std::string> put_obj_map(const std::string &title, const std::map<std::string, Hello> &map) {
  std::map<std::string, std::string> r;
  for (auto obj : map) {
    r.insert({obj.first, obj.second.Greet(title)});
  }
  return r;
}

void increment_critical(std::map<std::string, Critical *> v, int inc) {
  for (auto el : v) {
    el.second->Increment(inc);
  }
}

#include <nobind.h>

NOBIND_MODULE(map, m) {
  m.def<Hello>("Hello").cons<std::string &>().def<&Hello::Id>("get_id").def<&Hello::id, Nobind::ReadOnly>("id");
  m.def<Critical>("Critical").cons<>().def<&Critical::Get>("get");

  m.def<&get_ptr_map>("get_ptr_map");
  m.def<&put_ptr_map>("put_ptr_map");
  m.def<&get_obj_map>("get_obj_map");
  m.def<&put_obj_map>("put_obj_map");

  m.def<&increment_critical, Nobind::ReturnAsync>("incrementCritical");
}
