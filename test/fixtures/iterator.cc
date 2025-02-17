#include "iterator.h"

// The custom iterator implementation - nothing unusual here, it comes from:
// https://en.cppreference.com/w/cpp/iterator/iterator
//
// The JS magic is in the test
//
template <long FROM, long TO> Range<FROM, TO>::iterator::iterator(long _num = 0) : num(_num) {}

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
template <long FROM, long TO> long &Range<FROM, TO>::iterator::operator*() const { return num; }

template <long FROM, long TO> typename Range<FROM, TO>::iterator Range<FROM, TO>::begin() {
  return Range::iterator(FROM);
}
template <long FROM, long TO> typename Range<FROM, TO>::iterator Range<FROM, TO>::end() {
  return Range::iterator(TO >= FROM ? TO + 1 : TO - 1);
}
