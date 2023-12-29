#include <fixtures/qualifiers.h>

#include <nobind.h>

NOBIND_MODULE(qualifiers, m) {
  m.def<Qualified>("Qualified")
      .cons<>()
      .def<&Qualified::get1>("get1")
      .def<&Qualified::get2>("get2")
      .def<&Qualified::get3>("get3")
      .def<&Qualified::static_get4>("get4")
      .def<&Qualified::get1, Nobind::ReturnAsync>("get1Async")
      .def<&Qualified::get2, Nobind::ReturnAsync>("get2Async")
      .def<&Qualified::get3, Nobind::ReturnAsync>("get3Async")
      .def<&Qualified::static_get4, Nobind::ReturnAsync>("get4Async");

  m.def<&GlobalQualified>("globalQualified");
  m.def<&GlobalQualified, Nobind::ReturnAsync>("globalQualifiedAsync");
}
