#include <fixtures/nested.h>

#include <nobind.h>

NOBIND_MODULE(nested, m) {
  m.def<Time>("Time").cons<unsigned long>().def < &Time::operator unsigned long>("get");

  m.def<DateTime>("DateTime")
      .cons<Time>()
      .cons<unsigned long>()
      // Getters of object members references automatically return nested references
      .def<&DateTime::time>("time")
      // Explicitly return a nested reference
      .def<&DateTime::operator Time &, Nobind::ReturnNested>("get")
      // Explicitly return a nested pointer
      .def<&DateTime::operator Time *, Nobind::ReturnNested>("ptr");
}
