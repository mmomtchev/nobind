class Base {
protected:
  int b;

public:
  Base(int);
  virtual ~Base();
  virtual int get() const;
  int base_get() const;
};

class Derived : public Base {
protected:
  int d;

public:
  Derived(int);
  virtual ~Derived();
  virtual int get() const override;
  int derived_get() const;
};
