#include "basic_class.h"

Hello::Hello(const std::string &name) : name_(name), id(id_++){};
int Hello::Id() { return id; }
std::string Hello::Greet(const std::string &title){
  std::stringstream r;
  r << "hello " << title << " " << name_;
  return r.str();
}
int Hello::id_ = 0;
