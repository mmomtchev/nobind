#include <exception>
#include <fixtures/basic_class.h>
#include <fixtures/nested.h>

#include <nobind.h>

#include <vector>

constexpr auto NestedAsync = Nobind::ReturnNested | Nobind::ReturnAsync;

template <typename T> T &ReturnSame(T &v) { return v; }
constexpr auto *ReturnSameHello = &ReturnSame<Hello>;
constexpr auto *ReturnSameDT = &ReturnSame<DateTime>;

std::vector<DateTime> cpp_container;

#define TEST_SIZE 1000

DateTime &check_and_replace(std::vector<DateTime> &cpp_container, unsigned long element, bool replace) {
  // Check that this element is what we expect
  if ((unsigned long)cpp_container[element].time != element)
    throw std::logic_error{"Data corruption"};

  if (replace) {
    // Replace it with a new one
    cpp_container.at(element) = Time{element};
  }

  return cpp_container.at(element);
}

NOBIND_MODULE(stress, m) {
  m.def<Hello>("Hello").cons<std::string &>().def<&Hello::Id, Nobind::ReturnAsync>("get_id").ext<ReturnSameHello>(
      "same");

  m.def<Time>("Time").cons<unsigned long>().def<&Time::operator unsigned long, Nobind::ReturnAsync>("get");

  m.def<DateTime>("DateTime")
      .cons<Time>()
      .cons<unsigned long>()
      .def<&DateTime::operator Time &, NestedAsync>("get")
      .ext<ReturnSameDT>("same");

  m.def<&cpp_container>("cpp_container");

  cpp_container.resize(TEST_SIZE);
  for (size_t i = 0; i < TEST_SIZE; i++) {
    cpp_container.at(i) = Time{static_cast<unsigned long>(i)};
  }
  m.def<&check_and_replace, Nobind::ReturnShared>("cpp_check_and_replace");
}
