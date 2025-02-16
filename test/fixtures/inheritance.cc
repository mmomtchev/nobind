#include "inheritance.h"

Base::Base(int v) : b(v) {}
Base::~Base() {}
int Base::get() const { return b; }
int Base::base_get() const { return b; }

Derived::Derived(int v) : Base(v), d(v + 1) {}
Derived::~Derived() {}
int Derived::get() const { return d; }
int Derived::derived_get() const { return d; }

int IF1::ret1() { return 1; }
int IF2::ret2() { return 2; }

DerivedAbstract::DerivedAbstract(int v) : id(v) {}
std::string DerivedAbstract::GetName() { return "DerivedAbstract"; }
int DerivedAbstract::Id() { return id; }
