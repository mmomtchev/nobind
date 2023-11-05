#include <fixtures/basic_class.h>
#include <fixtures/global_functions.h>

#include <nobind.h>

NOBIND_MODULE(exceptions, m) {
  m.def<&throws>("throws")
    .def<Hello>("Hello")
      .cons<const std::string &>()
      .def<&Hello::throws>("throws");
}
