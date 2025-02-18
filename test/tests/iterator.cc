#include <fixtures/basic_class.h>
#include <fixtures/iterator.h>

#include <list>

#include <nobind.h>

// This is the tricky part

// There are two possible strategies illustrated with two examples.
// C++ iterators return a reference to the object in the container.
// * for scalar values and trivial classes the best option is to copy the object
//   returned to JS to avoid any memory management issues
// * for complex or non-copyable objects the only solution is to return the
//   reference itself and keep a reference to the container in the JS object

// Example 1: Iterating over scalar values
// (this includes iterating over object pointers)
using Iterable1 = Range<10, 20>;

// Example 2: Iterating over objects
using Iterable2 = std::list<Hello>;

// Example 3: Iterating over pointers (the objects are not owned by the container)
// (using Nobind::ReturnCopy will copy them, here we simply use the pointers)
using Iterable3 = std::list<Hello *>;

// For Iterable3, the built-in std::list::push_back won't work for us, nobind17 does not
// support rvalue references
void push_back_ptr(Iterable3 &list, Hello *el) { return list.push_back(el); }

// This is needed only to force MSVC from VS 2019 to instantiate the templates
constexpr auto *CopyIteratorIterable1 = &Nobind::MakeJSIterator<Iterable1, Nobind::ReturnCopy>;
constexpr auto *ReferenceIteratorIterable2 = &Nobind::MakeJSIterator<Iterable2, Nobind::ReturnNested>;
constexpr auto *SharedIteratorIterable3 = &Nobind::MakeJSIterator<Iterable3, Nobind::ReturnShared>;

NOBIND_MODULE(iterator, m) {
  m.def<Hello>("Hello").cons<const std::string &>().def<&Hello::Greet>("greet");

  // These classes do not inherit because Iterator and Iterable are not defined and must use void
  // as a parent class but they implement the TS Iterator<> and Iterable<> interfaces

  // Wrap the JS-compatible iterators to be exposed as abstract classes (no constructor) to JS
  // JS needs to know about their operator next() and the templates must be instantiated to be used
  // from JS as a C++ template can be instantiated only by the compiler - no runtime instantiation
  m.def<Nobind::JSIterator<Iterable1, Nobind::ReturnCopy>, void, Nobind::TSIterator<Iterable1>>(
       "_nobind_range_copy_iterator")
      .def<&Nobind::JSIterator<Iterable1, Nobind::ReturnCopy>::next>("next");
  m.def<Nobind::JSIterator<Iterable2, Nobind::ReturnNested>, void, Nobind::TSIterator<Iterable2>>(
       "_nobind_list_ref_iterator")
      .def<&Nobind::JSIterator<Iterable2, Nobind::ReturnNested>::next>("next");
  m.def<Nobind::JSIterator<Iterable3, Nobind::ReturnShared>, void, Nobind::TSIterator<Iterable3>>(
       "_nobind_list_shared_iterator")
      .def<&Nobind::JSIterator<Iterable3, Nobind::ReturnShared>::next>("next");

  // Expose the iterables to JS with a the helper that constructs a JS-compatible iterator
  // attached to [Symbol.iterator]
  m.def<Iterable1, void, Nobind::TSIterable<Iterable1>>("Range_10_20")
      .cons<>()
      .ext<CopyIteratorIterable1>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
  m.def<Iterable2, void, Nobind::TSIterable<Iterable2>>("HelloList")
      .cons<>()
      .def<static_cast<void (Iterable2::*)(const Hello &)>(&Iterable2::push_back)>("push_back")
      .ext<ReferenceIteratorIterable2>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
  m.def<Iterable3, void, Nobind::TSIterable<Iterable3>>("HelloPtrList")
      .cons<>()
      .ext<&push_back_ptr>("push_back")
      .ext<SharedIteratorIterable3>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
}
