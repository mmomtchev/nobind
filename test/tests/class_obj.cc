#include <fixtures/basic_class.h>

int hello_ref(Hello &h) {
  return h.id;
}

int hello_ptr(Hello *h) {
  return h->id;
}

int hello_const_ref(const Hello &h) {
  return h.id;
}

int hello_const_ptr(const Hello *h) {
  return h->id;
}


#include "nobind.h"

NOBIND_MODULE(basic_class, m) {
  m.def<Hello>("Hello")
    .cons<std::string &>();
  m.def<hello_ref>("hello_ref");
  m.def<hello_ptr>("hello_ptr");
  m.def<hello_const_ref>("hello_const_ref");
  m.def<hello_const_ptr>("hello_const_ptr");
}
