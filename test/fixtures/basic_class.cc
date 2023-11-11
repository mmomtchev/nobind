#include "basic_class.h"

Hello::Hello(const std::string &name) : name_(name), id(id_++) {}

int Hello::Id() { return id; }

std::string Hello::Greet(const std::string &title) {
  std::stringstream r;
  r << "hello " << title << " " << name_;
  return r.str();
}

void Hello::nothing() {}

void Hello::throws() {
  throw std::runtime_error("Hello error");
}

Hello *Hello::Factory(const std::string &name) {
  if (name.size() == 0)
    return nullptr;
  return new Hello(name);
}

Hello staticHello{"Forever young"};
const Hello &Hello::StaticHello() {
  return staticHello;
}

int Hello::id_ = 0;
