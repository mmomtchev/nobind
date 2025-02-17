#include <fixtures/iterator.h>

#include <nobind.h>

void next() {}
// This is the tricky part
Napi::Value iterate() {}

NOBIND_MODULE(iterator, m) {
  m.def<Range<10, 20>>("Range_10_20").cons().ext<iterate>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
}
