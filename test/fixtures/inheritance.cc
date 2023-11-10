#include "inheritance.h"

Base::Base(int v) : b(v) {}
Base::~Base() {}
int Base::get() const { return b; }
int Base::base_get() const { return b; }

Derived::Derived(int v) : Base(v), d(v + 1) {}
Derived::~Derived() {}
int Derived::get() const { return d; }
int Derived::derived_get() const { return d; }
