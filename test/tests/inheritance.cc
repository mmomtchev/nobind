#include <fixtures/abstract.h>
#include <fixtures/inheritance.h>

#include <nobind.h>

NOBIND_MODULE(inheritance, m) {
  m.def<IF1>("IF1").def<&IF1::ret1>("ret1");
  m.def<IF2>("IF2").def<&IF2::ret2>("ret2");
  m.def<Base>("Base").cons<int>().def<&Base::get>("get").def<&Base::base_get>("base_get");

  m.def<Derived, Base, IF1, IF2>("Derived")
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
      .def<&IF1::ret1>("ret1")
      .def<&IF2::ret2>("ret2")
#else
      .def<&Derived::get>("virtual_base_get")
      .def<static_cast<int (Derived::*)() const>(&Derived::base_get)>("base_get")
      .def<static_cast<int (Derived::*)() const>(&Derived::ret1)>("ret1")
      .def<static_cast<int (Derived::*)() const>(&Derived::ret2)>("ret2")
#endif
      .def<&Derived::derived_get>("derived_get");

  m.def<Abstract>("Abstract");
  m.def<DerivedAbstract, Abstract>("DerivedAbstract").cons<int>().def<&DerivedAbstract::Id>("id");

  m.def<&require_Base>("requireBase");
  // Automatic upcasting with multiple inheritance is still not supported
  // (it works only with the first class which is the JS parent)
  m.def<&require_IF1>("requireIF1");
  m.def<&return_Base>("returnBase");
}
