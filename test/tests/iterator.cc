#include <fixtures/iterator.h>

#include <nobind.h>

// This is the tricky part
template <typename T> class JSIterator {
  T &target;
  typename T::iterator it;

public:
  JSIterator(T &obj, typename T::iterator begin) : target(obj), it(begin) {}
  Napi::Value next(Napi::Env env) {
    Napi::Object ret;
    if (it == target.end()) {
      ret.Set("done", Napi::Boolean::New(env, "true"));
    } else {
      ret.Set("value", Napi::Number::New(env, *it));
      ret.Set("done", Napi::Boolean::New(env, "false"));
      it++;
    }
    return ret;
  }
};

template <typename T> JSIterator<T> IteratorWrapper(T &obj) { return JSIterator<T>{obj, obj.begin()}; }

NOBIND_MODULE(iterator, m) {
  Range<10, 20> a;

  // m.def<JSIterator<Range<10, 20>>>("Range_10_20_iterator").def<&JSIterator<Range<10, 20>>::next>("next");

  // m.def<Range<10, 20>>("Range_10_20")
  //.cons<int>()
  //.ext<&IteratorWrapper<Range<10, 20>>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
}
