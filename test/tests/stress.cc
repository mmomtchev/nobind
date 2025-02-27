#include <exception>
#include <fixtures/basic_class.h>
#include <fixtures/nested.h>

#include <nobind.h>

constexpr auto NestedAsync = Nobind::ReturnNested | Nobind::ReturnAsync;

template <typename T> T &ReturnSame(T &v) { return v; }
constexpr auto *ReturnSameHello = &ReturnSame<Hello>;
constexpr auto *ReturnSameDT = &ReturnSame<DateTime>;

NOBIND_MODULE(stress, m) {
  m.def<Hello>("Hello").cons<std::string &>().def<&Hello::Id, Nobind::ReturnAsync>("get_id").ext<ReturnSameHello>(
      "same");

  m.def<Time>("Time").cons<unsigned long>().def<&Time::operator unsigned long, Nobind::ReturnAsync>("get");

  m.def<DateTime>("DateTime")
      .cons<Time>()
      .cons<unsigned long>()
      .def<&DateTime::operator Time &, NestedAsync>("get")
      .ext<ReturnSameDT>("same");
}
