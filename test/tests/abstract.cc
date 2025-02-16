#include <fixtures/basic_class.h>
#include <fixtures/undestructible.h>

#include <nobind.h>

NOBIND_MODULE(undescrutible, m) {
  static_assert(!std::is_destructible_v<Undestructible>, "Undestructible is destructible");
  m.def<Undestructible>("Undestructible").cons<>();

  // Defining a class w/o JS constructor creates an abstract class
  m.def<Hello>("Unconstructible");
}
