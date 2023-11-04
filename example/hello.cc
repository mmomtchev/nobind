#include <cmath>
#include <iostream>
#include <sstream>

int add(int a, int b) { return a + b; }

bool gte(int a, int b) { return a >= b; }

double power(double a, double b) { return std::pow(a, b); }

std::string hello(const std::string &s) {
  std::stringstream r;
  r << "hello " << s;
  return r.str();
}

class Hello {
  static int id_;
  std::string name_;
  int id;

public:
  Hello(const std::string &name) : name_(name), id(id_++){};
  int Id() { return id; }
  std::string Greet() {
    std::stringstream r;
    r << "hello " << name_;
    return r.str();
  }
};
int Hello::id_ = 0;

#include "nobind.h"

NOBIND_MODULE(hello, m) {
  m.def<&add>("add");
  m.def<&gte>("gte");
  m.def<&power>("pow");
  m.def<&hello>("hello");
  m.def<Hello>("Hello")
    .cons<std::string &>()
    .def<&Hello::Id>("id")
    .def<&Hello::Greet>("greet");
}
