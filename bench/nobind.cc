#include "string.h"

#include <nobind.h>

NOBIND_MODULE(bench_nobind_string, m) {
  m.def<String>("String")
    .cons<std::string &>()
    .def<&String::Len>("length");
  m.def<&Strlen>("strlen");
}
