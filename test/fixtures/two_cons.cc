#include "two_cons.h"

TwoCons::TwoCons(int a) {
  x = a;
}

TwoCons::TwoCons(const std::string &s) {
  x = std::stoi(s);
}

TwoCons::TwoCons() {
 x = -1;
}
