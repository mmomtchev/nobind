#pragma once
#include <string>

class Abstract {
public:
  virtual std::string GetName();
  virtual int Id() = 0;

protected:
  virtual ~Abstract() = default;
};
