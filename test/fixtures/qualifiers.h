class Qualified {
public:
  constexpr Qualified() noexcept {};
  virtual ~Qualified();
  constexpr int get1() const { return 1; };
  virtual int get2() noexcept;
  int get3() const noexcept;
  constexpr static int static_get4() noexcept { return 4; };
};

constexpr int GlobalQualified() noexcept { return 0; };
