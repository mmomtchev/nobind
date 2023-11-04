#include <fixtures/global_functions.h>

#include "nobind.h"

NOBIND_MODULE(global_functions, m) {
  m.def<&add>("add");
  m.def<&gte>("gte");
  m.def<&power>("pow");
  m.def<&hello>("hello");
}
