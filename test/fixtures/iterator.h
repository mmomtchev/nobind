// Range is a standard C++17 iterable object
//
// Can be used from C++ as:
//
// for (long l : Range<3, 5>())
//   std::cout << l << ' '; // 3 4 5
//

#include <algorithm>
#include <iostream>

template <long FROM, long TO> class Range {
  int d;

public:
  explicit Range();
  class iterator {
    long num = FROM;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = long;
    using difference_type = long;
    using pointer = long *;
    using reference = long &;

    explicit iterator(long _num = 0);
    iterator &operator++();
    iterator operator++(int);
    bool operator==(iterator other) const;
    bool operator!=(iterator other) const;
    long &operator*() const;
  };
  iterator begin();
  iterator end();
};
