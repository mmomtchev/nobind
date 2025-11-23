#include <cmath>
#include <iostream>
#include <sstream>

#include "hello.h"

int add(int a, int b) { return a + b; }

bool gte(int a, int b) { return a >= b; }

double power(double a, double b) { return std::pow(a, b); }

std::string hello(const std::string &s) {
  std::stringstream r;
  r << "hello " << s;
  return r.str();
}

Hello::Hello() : id(id_++) {}
Hello::Hello(const std::string &name) : name_(name), id(id_++) {}
int Hello::Id() { return id; }
std::string Hello::Greet() {
  std::stringstream r;
  r << "hello " << name_;
  return r.str();
}
int Hello::id_ = 0;

don't build
