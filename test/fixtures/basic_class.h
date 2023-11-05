#include <sstream>

class Hello {
  static int id_;
  std::string name_;

public:
  int id;
  int var;
  Hello(const std::string &name);
  int Id();
  std::string Greet(const std::string &);
  void nothing();
  void throws();
};
