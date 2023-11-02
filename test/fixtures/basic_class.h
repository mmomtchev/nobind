#include <sstream>

class Hello {
  static int id_;
  std::string name_;
  int id;

public:
  Hello(const std::string &name);
  int Id();
  std::string Greet(const std::string &);
};
