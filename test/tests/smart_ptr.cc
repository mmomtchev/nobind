#include <fixtures/basic_class.h>

#include <nobind.h>

#include <memory>

int take_shared_ptr(std::shared_ptr<Hello> in) { return in->Id(); }

std::shared_ptr<Hello> return_shared_ptr(const std::string &name) {
  return std::shared_ptr<Hello>(Hello::Factory(name));
}

NOBIND_MODULE(basic_class, m) {
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id>("get_id")
      .def<&Hello::Greet>("greet")
      .def<&Hello::id, Nobind::ReadOnly>("id")
      .def<&Hello::Factory>("factory");

  m.def<&take_shared_ptr>("takeSharedPtr");
}
