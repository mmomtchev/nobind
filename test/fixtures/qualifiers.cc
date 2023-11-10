#include "qualifiers.h"

Qualified::~Qualified() {};
int Qualified::get2() noexcept { return 2; };
int Qualified::get3() const noexcept { return 3; };
