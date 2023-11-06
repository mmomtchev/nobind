#include "chaining.h"

Chained::Chained() : var(0) {}

Chained &Chained::Inc1() {
  var += 1;
  return *this;
}
Chained &Chained::Inc10() {
  var += 10;
  return *this;
}
Chained &Chained::Inc100() {
  var += 100;
  return *this;
}

int Chained::Get() {
  return var;
}
