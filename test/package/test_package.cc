void test_function();

#include <nobind.h>

// Define a new module
NOBIND_MODULE(test_package, m) {
  m.def<test_function>("test_function");
}
