#include <fixtures/basic_class.h>

#include <nobind.h>

NOBIND_MODULE(basic_class, m) {
  m.def<Hello>("Hello")
    .cons<std::string &>()
    .def<&Hello::Id>("get_id")
    .def<&Hello::Greet>("greet")
    .def<&Hello::nothing>("nothing")
    .def<&Hello::id, Nobind::readOnly>("id")
    .def<&Hello::id>("var")
    .def<&Hello::Factory>("factory");
}
