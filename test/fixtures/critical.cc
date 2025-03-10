#include "critical.h"

Critical::Critical() : counter(0) {}
void Critical::Increment(int v) {
  for (int i = 0; i < v; i++)
    counter++;
}
int Critical::Get() { return counter; }
