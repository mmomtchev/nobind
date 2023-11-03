#include <fixtures/basic_class.h>

int hello_ref(Hello &h) {
  return h.id;
}

int hello_ptr(Hello *h) {
  return h->id;
}

#include "nobind.h"

NOBIND_MODULE(basic_class, m) {
  m.def<Hello>("Hello")
    .cons<std::string &>();
  m.def<hello_ref>("hello_ref");
  m.def<hello_ptr>("hello_ptr");
}
