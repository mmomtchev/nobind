#include <fixtures/two_cons.h>

#include <nobind.h>

NOBIND_MODULE(cons_overload, m) {
  m.def<TwoCons>("TwoCons").cons<>().cons<const std::string &>().cons<int>().cons<bool>().def<&TwoCons::x>("x");
}
