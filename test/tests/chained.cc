#include <fixtures/chaining.h>

#include <nobind.h>

NOBIND_MODULE(chained, m) {
  m.def<Chained>("Chained")
    .cons<>()
    .def<&Chained::Inc1>("inc1")
    .def<&Chained::Inc10>("inc10")
    .def<&Chained::Inc100>("inc100")
    .def<&Chained::Get>("get");
}
