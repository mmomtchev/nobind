#include "dependant_class.h"

Dependant::Dependant(const Hello &hello) : hello_(hello) {}

const Hello &Dependant::Get() const { return hello_; }
