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
  int ret1();
};

class IF2 {
public:
  int ret2();
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
