#include <fixtures/chaining.h>

#include <nobind.h>

NOBIND_MODULE(chained, m) {
  m.def<Chained>("Chained")
    .cons<>()
    .def<&Chained::Inc1, Nobind::ReturnShared>("inc1")
    .def<&Chained::Inc10, Nobind::ReturnShared>("inc10")
    .def<&Chained::Inc100, Nobind::ReturnShared>("inc100")
    .def<&Chained::Get>("get");
}
