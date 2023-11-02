#include "global_functions.h"

int add(int a, int b) { return a + b; }

bool gte(int a, int b) { return a >= b; }

double power(double a, double b) { return std::pow(a, b); }

std::string hello(const std::string &s) {
  std::stringstream r;
  r << "hello " << s;
  return r.str();
}
