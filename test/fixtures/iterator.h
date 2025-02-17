// Range is a standard C++ iterable object
// https://en.cppreference.com/w/cpp/iterator/iterator
//
//
// Can be used from C++ as:
//
// for (long l : Range<3, 5>())
//   std::cout << l << ' '; // 3 4 5
//

#include <algorithm>
#include <iostream>

template <long FROM, long TO> class Range {
public:
  class iterator : public std::iterator<std::input_iterator_tag, long, long, const long *, long> {
    long num = FROM;

  public:
    explicit iterator(long _num = 0) : num(_num) {}
    iterator &operator++() {
      num = TO >= FROM ? num + 1 : num - 1;
      return *this;
    }
    iterator operator++(int) {
      iterator retval = *this;
      ++(*this);
      return retval;
    }
    bool operator==(iterator other) const { return num == other.num; }
    bool operator!=(iterator other) const { return !(*this == other); }
    long &operator*() const { return num; }
  };
  iterator begin() { return iterator(FROM); }
  iterator end() { return iterator(TO >= FROM ? TO + 1 : TO - 1); }
};
