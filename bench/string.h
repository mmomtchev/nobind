#include <string>

size_t Strlen(const std::string &s);

class String {
  std::string s;

public:
  String(const std::string &init);
  size_t Len();
};
