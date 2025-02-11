#include "global_functions.h"

int add(int a, int b) { return a + b; }

bool gte(int a, int b) { return a >= b; }

int testa(bool a) { return a ? 1 : 0; }

double power(double a, double b) { return std::pow(a, b); }

std::string hello(const std::string &s) {
  std::stringstream r;
  r << "hello " << s;
  return r.str();
}

void nothing() {}

void throws() { throw std::runtime_error("Global error"); }
