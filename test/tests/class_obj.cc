#include <fixtures/basic_class.h>
#include <fixtures/pod_class.h>
#include <fixtures/two_cons.h>

int hello_ref(Hello &h) { return h.id; }

int hello_ptr(Hello *h) { return h->id; }

int hello_const_ref(const Hello &h) { return h.id; }

int hello_const_ptr(const Hello *h) { return h->id; }

int hello_pod(IntObject v) { return v.value; }

#include <nobind.h>

constexpr auto strictAsync = Nobind::ReturnAsync | Nobind::ReturnNullThrow;

NOBIND_MODULE(class_obj, m) {
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Factory, Nobind::ReturnNullAccept>("factory_tolerant")
      .def<&Hello::Factory, Nobind::ReturnNullThrow>("factory_strict")
      .def<&Hello::Factory, strictAsync>("factoryAsync_strict");
  m.def<&hello_ref>("hello_ref");
  m.def<&hello_ptr>("hello_ptr");
  m.def<&hello_const_ref>("hello_const_ref");
  m.def<&hello_const_ptr>("hello_const_ptr");

  m.def<IntObject>("IntObject").cons<int>();
  m.def<&hello_pod>("hello_pod");

  m.def<TwoCons>("TwoCons").cons<>();
}
