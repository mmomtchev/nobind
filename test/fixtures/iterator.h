// Range is a standard C++17 iterable object
//
// Can be used from C++ as:
//
// for (long l : Range<3, 5>())
//   std::cout << l << ' '; // 3 4 5
//

#include <iterator>

template <long FROM, long TO> class Range {
  int d;

public:
  explicit Range();
  class iterator {
    long num = FROM;

  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = long;
    using difference_type = long;
    using pointer = long *;
    using reference = long &;

    explicit iterator(long _num = 0);
    iterator &operator++();
    iterator operator++(int);
    bool operator==(iterator other) const;
    bool operator!=(iterator other) const;
    long &operator*();
  };
  iterator begin();
  iterator end();
};

//
// The custom iterator implementation - nothing unusual here
// The JS magic is in the test
//
template <long FROM, long TO> Range<FROM, TO>::iterator::iterator(long _num) : num(_num) {}

template <long FROM, long TO> typename Range<FROM, TO>::iterator &Range<FROM, TO>::iterator::operator++() {
  num = TO >= FROM ? num + 1 : num - 1;
  return *this;
}
template <long FROM, long TO> typename Range<FROM, TO>::iterator Range<FROM, TO>::iterator::operator++(int) {
  iterator retval = *this;
  ++(*this);
  return retval;
}
template <long FROM, long TO> bool Range<FROM, TO>::iterator::operator==(iterator other) const {
  return num == other.num;
}
template <long FROM, long TO> bool Range<FROM, TO>::iterator::operator!=(iterator other) const {
  return !(*this == other);
}
template <long FROM, long TO> long &Range<FROM, TO>::iterator::operator*() { return num; }

template <long FROM, long TO> typename Range<FROM, TO>::iterator Range<FROM, TO>::begin() {
  return Range::iterator(FROM);
}
template <long FROM, long TO> typename Range<FROM, TO>::iterator Range<FROM, TO>::end() {
  return Range::iterator(TO >= FROM ? TO + 1 : TO - 1);
}
template <long FROM, long TO> Range<FROM, TO>::Range() {}
