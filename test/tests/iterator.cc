#include <fixtures/iterator.h>

#include <nobind.h>

// This is the tricky part
// This is a JS iterator
// You can use it as an example as to how to implement your own iterators
// It is generic enough to be directly usable with all types,
// but you must understand the implications of returning references to JS
template <typename T> class JSIterator {
  T &target;
  typename T::iterator it;

public:
  JSIterator(T &obj, typename T::iterator begin) : target(obj), it(begin) {}
  Napi::Value next(Napi::Env env) {
    using value_type_t = typename decltype(it)::value_type;

    Napi::Object ret = Napi::Object::New(env);
    if (it == target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      Napi::Value value;
      if constexpr (std::is_scalar_v<value_type_t>) {
        // Scalar types are simply copied
        value = Nobind::ToJS<value_type_t, Nobind::ReturnOwned>(env, *it).Get();
      } else if constexpr (std::is_copy_constructible_v<value_type_t>) {
        // If the object is copy-constructible the safest option is to copy it
        value = Nobind::ToJS<value_type_t, Nobind::ReturnOwned>(env, *it).Get();
      } else {
        // Other objects are returned as a shared reference
        // This assumes that their container won't get destroyed
        // Note that ReturnNested won't work here - it is valid only for class members
        value = Nobind::ToJS<value_type_t &, Nobind::ReturnShared>(env, *it).Get();
      }
      ret.Set("value", value);
      ret.Set("done", Napi::Boolean::New(env, false));
      it++;
    }
    return ret;
  }
};

using Iterable = Range<10, 20>;

// Helper to construct it
// MSVC 2019 is not fully C++17 compliant and has problems with using
// pointers to templated functions as non-type template arguments
#if !defined(_MSC_VER) || _MSC_VER >= 1930
template <typename T> JSIterator<T> IteratorWrapper(T &obj) { return JSIterator<T>{obj, obj.begin()}; }
#else
JSIterator<Iterable> IteratorWrapper(Iterable &obj) { return JSIterator<Iterable>{obj, obj.begin()}; }
#endif

NOBIND_MODULE(iterator, m) {
  m.def<JSIterator<Iterable>>("Range_10_20_iterator").def<&JSIterator<Iterable>::next>("next");

#if !defined(_MSC_VER) || _MSC_VER >= 1930
  m.def<Iterable>("Range_10_20").cons<>().ext<&IteratorWrapper<Iterable>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
#else
  m.def<Iterable>("Range_10_20").cons<>().ext<&IteratorWrapper>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
#endif
}
