template <typename T> class Template {
  T member;

public:
  Template(T arg) : member(arg){};
  T getter() { return member; };
};
