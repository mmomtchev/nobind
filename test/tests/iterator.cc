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
    Napi::Object ret = Napi::Object::New(env);
    if (it == target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      auto value = Nobind::ToJS<std::remove_reference_t<decltype(*it)>, Nobind::ReturnDefault>(env, *it);
      ret.Set("value", value.Get());
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
