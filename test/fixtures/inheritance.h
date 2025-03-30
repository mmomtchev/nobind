#include "abstract.h"

class Base {
protected:
  int b;

public:
  Base(int);
  virtual ~Base();
  virtual int get() const;
  int base_get() const;
};

class IF1 {
public:
  int ret1() const;
};

class IF2 {
public:
  int ret2() const;
};

class Derived : public Base, public IF1, public IF2 {
protected:
  int d;

public:
  Derived(int);
  virtual ~Derived();
  virtual int get() const override;
  int derived_get() const;
};

class DerivedAbstract : public Abstract {
  int id;

public:
  DerivedAbstract(int);
  virtual ~DerivedAbstract() override = default;
  virtual std::string GetName() override;
  virtual int Id() override;
};

int require_Base(const Base &);
int require_Derived(const Derived &);
int require_IF1(const IF1 &);
const Base &return_Base(const Base &);
