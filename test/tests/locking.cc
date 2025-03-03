#include <fixtures/critical.h>

#include <nobind.h>

NOBIND_MODULE(locking, m) {
  m.def<Critical>("Critical")
      .cons<>()
      .def<&Critical::Increment, Nobind::ReturnAsync>("increment")
      .def<&Critical::Get>("get");
}
