#include <fixtures/inheritance.h>

#include <nobind.h>

NOBIND_MODULE(inheritance, m) {
  m.def<Base>("Base").cons<int>().def<&Base::get>("get").def<&Base::base_get>("base_get");

  m.def<Derived>("Derived")
      .cons<int>()
      .def<&Derived::get>("get")
      // Get explicitly the address to the virtual method in the Base class
      // The C++ templating will still invoke the correct method as the
      // invocation happens through a Derived object
      .def<&Base::get>("virtual_base_get")
      .def<&Derived::base_get>("base_get")
      .def<&Derived::derived_get>("derived_get");
}
