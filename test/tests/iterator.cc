#include <fixtures/basic_class.h>
#include <fixtures/iterator.h>

#include <list>

#include <nobind.h>

// This is the tricky part
// These are the JS iterators
// There are two possible strategies illustrated with two examples
// C++ iterators return a reference to the object in the container
// * for scalar values and trivial classes the best option is to copy the object
//   returned to JS to avoid any memory management issues
// * for complex or non-copyable objects the only solution is to return the
//   reference itself and keep a reference to the container in the JS object

// This is the copying iterator
template <typename T> class JSIteratorCopy {
  T &target;
  typename T::iterator it;

public:
  JSIteratorCopy(T &obj, const typename T::iterator &begin) : target(obj), it(begin) {}
  Napi::Value next(Napi::Env env) {
    using value_type_t = typename T::iterator::value_type;
    static_assert(std::is_copy_constructible_v<value_type_t>,
                  "JSIteratorCopy works only with copy-constructible objects");

    Napi::Object ret = Napi::Object::New(env);
    if (it == target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      ret.Set("value", Nobind::ToJS<value_type_t, Nobind::ReturnOwned>(env, *it).Get());
      ret.Set("done", Napi::Boolean::New(env, false));
      it++;
    }
    return ret;
  }
};

// This is the shared reference iterator
template <typename T> class JSIteratorReference {
  T &target;
  typename T::iterator it;
  // The iterator keeps a JS reference to his container to protect it
  // from the GC
  Napi::Reference<Napi::Value> persistent;

public:
  JSIteratorReference(T &) = delete;
  JSIteratorReference(T &obj, const typename T::iterator &begin, Napi::Value jsobj)
      : target(obj), it(begin), persistent(Napi::Persistent(jsobj)) {}
  Napi::Value next(Napi::Env env) {
    using value_type_t = typename T::iterator::value_type;
    static_assert(!std::is_scalar_v<value_type_t>, "JSIteratorReference should not be used with scalar values");

    Napi::Object ret = Napi::Object::New(env);
    if (it == target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      Napi::Value value = Nobind::ToJS<value_type_t &, Nobind::ReturnShared>(env, *it).Get();
      // Every returned object receives a copy of the reference to his container
      // Thus, the container can be destroyed only after all returned objects and
      // the iterator have been GCed
      value.ToObject().DefineProperty(
          Napi::PropertyDescriptor::Value(Napi::String::New(env, NOBIND_PARENT_PROP), persistent.Value()));
      ret.Set("value", value);
      ret.Set("done", Napi::Boolean::New(env, false));
      it++;
    }
    return ret;
  }
};

// Example 1: Iterating over scalar values
// (this includes iterating over object pointers)
using Iterable1 = Range<10, 20>;

// Example 2: Iterating over objects
using Iterable2 = std::list<Hello>;

// Helpers to construct the iterators
// The safe iterator does not need anything
template <typename T> JSIteratorCopy<T> IteratorCopyWrapper(T &obj) { return JSIteratorCopy<T>{obj, obj.begin()}; }

// The "dangerous" iterator keeps a JS reference to the container to protect it from the GC
// This iterator is not copy-constructible and must be allocated on the heap
// (Nobind will recognize the pointer type and it will take care of the destruction)
template <typename T> JSIteratorReference<T> *IteratorReferenceWrapper(Napi::Value jsobj) {
  // Unwrap the JS value to get the C++ object
  T &obj = Nobind::FromJSValue<T &>(jsobj).Get();
  return new JSIteratorReference<T>{obj, obj.begin(), jsobj};
}

NOBIND_MODULE(iterator, m) {
  m.def<Hello>("Hello").cons<const std::string &>().def<&Hello::Greet>("greet");

  // Wrap the JS-compatible iterators to be exposed as abstract classes (no constructor) to JS
  // JS needs to know about their operator next() and the templates must be instantiated to be used
  // from JS as a C++ template can be instantiated only by the compiler - no runtime instantiation
  m.def<JSIteratorCopy<Iterable1>>("_nobind_range_copy_iterator").def<&JSIteratorCopy<Iterable1>::next>("next");
  m.def<JSIteratorReference<Iterable2>>("_nobind_list_ref_iterator").def<&JSIteratorReference<Iterable2>::next>("next");

  // Expose the iterables to JS with a the helper that constructs a JS-compatible iterator
  // attached to [Symbol.iterator]
  m.def<Iterable1>("Range_10_20")
      .cons<>()
      .ext<&IteratorCopyWrapper<Iterable1>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
  m.def<Iterable2>("HelloList")
      .cons<>()
      .def<static_cast<void (Iterable2::*)(const Hello &)>(&Iterable2::push_back)>("push_back")
      .ext<&IteratorReferenceWrapper<Iterable2>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
}
