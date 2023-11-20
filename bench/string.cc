#include "string.h"

size_t Strlen(const std::string &s) { return s.size(); }

String::String(const std::string &init) : s(init){};
size_t String::Len() { return s.size(); }
