#include <fixtures/critical.h>

#include <nobind.h>

void Increment(Critical &o, int i) { return o.Increment(i); }

NOBIND_MODULE(locking, m) {
  m.def<Critical>("Critical")
      .cons<>()
      .def<&Critical::Increment, Nobind::ReturnAsync>("increment")
      .def<&Critical::Get>("get")
      .def<&Critical::counter>("value")
      .ext<&Increment>("ext");
  m.def<&Increment, Nobind::ReturnAsync>("increment");
}
