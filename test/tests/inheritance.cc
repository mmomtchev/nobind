#include <fixtures/inheritance.h>

#include <nobind.h>

NOBIND_MODULE(inheritance, m) {
  m.def<Base>("Base").cons<int>().def<&Base::get>("get").def<&Base::base_get>("base_get");

  m.def<Derived>("Derived")
      .cons<int>()
      .def<&Derived::get>("get")
  // Both of these are artificial edge cases for testing purposes
  // and MSVC 2019 is already pushed to its limits, we choose to certify
  // it without them as they are not really needed
#if !defined(_MSC_VER) || _MSC_VER >= 1930
      // Get explicitly the address to the virtual method in the Base class
      // The C++ templating will still invoke the correct method as the
      // invocation happens through a Derived object
      .def<&Base::get>("virtual_base_get")
      .def<&Derived::base_get>("base_get")
#else
      .def<&Derived::get>("virtual_base_get")
      .def<static_cast<int (Derived::*)() const>(&Derived::get)>("base_get")
#endif
      .def<&Derived::derived_get>("derived_get");
}
