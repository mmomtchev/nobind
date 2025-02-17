#include "basic_class.h"

class Dependant {
  Hello hello_;

public:
  Dependant(const Hello &);
  const Hello &Get() const;
};
