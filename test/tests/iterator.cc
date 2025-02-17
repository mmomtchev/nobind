#include <fixtures/iterator.h>

#include <nobind.h>

// This is the tricky part
// This is a JS iterator
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
        value = Nobind::ToJS<value_type_t, Nobind::ReturnDefault>(env, *it).Get();
      } else {
        // Non-scalar types are returned as a shared reference
        // This assumes that their container won't get destroyed
        // If the object is copy-constructible the safest option is to copy it
        // Otherwise, special care must be taken that these objects contain a reference
        // to their container
        value = Nobind::ToJS<value_type_t &, Nobind::ReturnShared>(env, *it).Get();
      }
      ret.Set("value", value);
      ret.Set("done", Napi::Boolean::New(env, false));
      it++;
    }
    return ret;
  }
};

// Helper to construct it
template <typename T> JSIterator<T> IteratorWrapper(T &obj) { return JSIterator<T>{obj, obj.begin()}; }

NOBIND_MODULE(iterator, m) {
  using Iterable = Range<10, 20>;

  m.def<JSIterator<Iterable>>("Range_10_20_iterator").def<&JSIterator<Iterable>::next>("next");

  m.def<Iterable>("Range_10_20").cons<>().ext<&IteratorWrapper<Iterable>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
}
