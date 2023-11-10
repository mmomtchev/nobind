#include <fixtures/inheritance.h>

#include <nobind.h>

NOBIND_MODULE(inheritance, m) {
  m.def<Base>("Base")
    .cons<int>()
    .def<&Base::get>("get")
    .def<&Base::base_get>("base_get");

  m.def<Derived>("Derived")
    .cons<int>()
    .def<&Derived::get>("get")
    .def<&Derived::base_get>("base_get")
    .def<&Derived::derived_get>("derived_get");
}
