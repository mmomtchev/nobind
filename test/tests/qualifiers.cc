#include <fixtures/qualifiers.h>

#include <nobind.h>

NOBIND_MODULE(qualifiers, m) {
  m.def<Qualified>("Qualified")
    .cons<>()
    .def<&Qualified::get1>("get1")
    .def<&Qualified::get2>("get2")
    .def<&Qualified::get3>("get3");
}
