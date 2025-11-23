#pragma once
#include <string>

class Hello {
  static int id_;
  std::string name_;
  int id;

public:
  Hello();
  Hello(const std::string &name);
  int Id();
  std::string Greet();
};
