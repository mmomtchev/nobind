#include "two_cons.h"
#include <stdexcept>

TwoCons::TwoCons(int a) { x = a; }

TwoCons::TwoCons(const std::string &s) { x = std::stoi(s); }

TwoCons::TwoCons() { x = -1; }

TwoCons::TwoCons(bool) { throw std::logic_error{"wrong constructor"}; }
