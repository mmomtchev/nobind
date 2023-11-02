#include <fixtures/basic_class.h>

#include "nobind.h"

NOBIND_MODULE(basic_class, m) {
  m.def<Hello>("Hello")
    .cons<std::string &>()
    .def<&Hello::Id>("id")
    .def<&Hello::Greet>("greet");
}
