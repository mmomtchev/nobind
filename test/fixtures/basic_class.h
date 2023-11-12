#include <sstream>

class Hello {
  static int id_;
  std::string name_;

public:
  int id;
  int var;
  Hello(const std::string &name);
  int Id() const;
  std::string Greet(const std::string &);
  void nothing() const;
  void throws();

  static Hello *Factory(const std::string &name);
  static const Hello &StaticHello();
};
