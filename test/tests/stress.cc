#include <exception>
#include <fixtures/basic_class.h>
#include <fixtures/nested.h>

#include <nobind.h>

#include <memory>
#include <queue>
#include <vector>

constexpr auto NestedAsync = Nobind::ReturnNested | Nobind::ReturnAsync;
constexpr auto SharedAsync = Nobind::ReturnShared | Nobind::ReturnAsync;

template <typename T> T &ReturnSame(T &v) { return v; }
constexpr auto *ReturnSameHello = &ReturnSame<Hello>;
constexpr auto *ReturnSameDT = &ReturnSame<DateTime>;

std::vector<DateTime> cpp_container;

#define TEST_SIZE 1000

DateTime &check_and_replace(unsigned long element, bool replace) {
  // Check that this element is what we expect
  if ((unsigned long)cpp_container[element].time != element)
    throw std::logic_error{"Data corruption"};

  if (replace) {
    // Replace it with a new one
    cpp_container.at(element) = Time{element};
  }

  return cpp_container.at(element);
}

std::vector<Hello> take_and_return_object_vector(const std::vector<Hello> &input) {
  std::vector<Hello> output;
  for (auto i : input) {
    output.push_back(i);
  }
  return output;
}

std::vector<Hello *> take_and_return_ptr_vector(const std::vector<Hello *> &input) {
  std::vector<Hello *> output;
  for (auto i : input) {
    output.push_back(i);
  }
  return output;
}

std::shared_ptr<Hello> take_and_return_shared_ptr(const std::shared_ptr<Hello> in) { return in; }
std::shared_ptr<Hello> make_shared_ptr(std::string name) { return std::make_shared<Hello>(name); }

// https://github.com/mmomtchev/nobind/issues/56
std::queue<std::shared_ptr<Hello>> smart_ptr_collection;
std::mutex smart_ptr_collection_lock;
void take_and_keep_100_shared_ptr(std::shared_ptr<Hello> in) {
  std::lock_guard lock{smart_ptr_collection_lock};
  smart_ptr_collection.push(in);
  // Keep only 100 pointers and destroy when above the limit
  // This runs in a background thread
  while (smart_ptr_collection.size() > 100)
    smart_ptr_collection.pop();
}
std::vector<std::string> return_kept_shared_ptr() {
  std::vector<std::string> r;
  std::lock_guard lock{smart_ptr_collection_lock};
  while (!smart_ptr_collection.empty()) {
    r.push_back(smart_ptr_collection.front()->Greet("Citizen"));
    smart_ptr_collection.pop();
  }
  return r;
}

NOBIND_MODULE(stress, m) {
  m.def<Hello>("Hello")
      .cons<std::string &>()
      .def<&Hello::Id, Nobind::ReturnAsync>("get_id")
      .def<&Hello::Greet>("greet")
      .ext<ReturnSameHello>("same");

  m.def<Time>("Time").cons<unsigned long>().def<&Time::operator unsigned long, Nobind::ReturnAsync>("get");

  m.def<DateTime>("DateTime")
      .cons<Time>()
      .cons<unsigned long>()
      .def<&DateTime::operator Time &, NestedAsync>("get")
      .ext<ReturnSameDT>("same");

  cpp_container.resize(TEST_SIZE);
  for (size_t i = 0; i < TEST_SIZE; i++) {
    cpp_container.at(i) = Time{static_cast<unsigned long>(i)};
  }
  m.def<&check_and_replace, Nobind::ReturnShared>("cpp_check_and_replace");

  m.def<&take_and_return_object_vector, Nobind::ReturnAsync>("take_and_return_object_vector");
  // Without the object store this function is quite dangerous
  // (returned pointers are owned by default)
  m.def<&take_and_return_ptr_vector, SharedAsync>("take_and_return_ptr_vector");

  m.def<&take_and_return_shared_ptr, Nobind::ReturnAsync>("take_and_return_shared_ptr");
  m.def<&make_shared_ptr, Nobind::ReturnAsync>("make_shared_ptr");

  // https://github.com/mmomtchev/nobind/issues/56
  m.def<&take_and_keep_100_shared_ptr, Nobind::ReturnAsync>("take_and_keep_100_shared_ptr");
  m.def<&return_kept_shared_ptr>("return_kept_shared_ptr");
}
