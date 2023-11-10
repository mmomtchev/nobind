class Qualified {
public:
  constexpr Qualified() noexcept {};
  virtual ~Qualified();
  constexpr int get1() const { return 1; };
  virtual int get2() noexcept;
  int get3() const noexcept;
};
