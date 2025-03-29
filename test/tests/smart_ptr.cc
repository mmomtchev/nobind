#include <fixtures/basic_class.h>

#include <nobind.h>

#include <memory>

int take_shared_ptr(std::shared_ptr<Hello> in) { return in->Id(); }

int take_const_shared_ptr(std::shared_ptr<const Hello> in) { return in->Id(); }

std::shared_ptr<Hello> return_shared_ptr(const std::string &name) {
  return std::shared_ptr<Hello>(Hello::Factory(name));
}

NOBIND_MODULE(smart_ptr, m) {
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id>("get_id")
      .def<&Hello::Greet>("greet")
      .def<&Hello::id, Nobind::ReadOnly>("id")
      .def<&Hello::Factory>("factory");

  m.def<&take_shared_ptr>("takeSharedPtr");
  m.def<&take_const_shared_ptr>("takeConstSharedPtr");
  m.def<&return_shared_ptr>("returnSharedPtr");
}
