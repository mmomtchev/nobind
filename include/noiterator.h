// These are the JS iterators
// There are two possible strategies illustrated with two examples.
// C++ iterators return a reference to the object in the container.
// * for scalar values and trivial classes the best option is to copy the object
//   returned to JS to avoid any memory management issues
// * for complex or non-copyable objects the only solution is to return the
//   reference itself and keep a reference to the container in the JS object

#include <notypes.h>

namespace Nobind {

// An IteratorResult is a Napi::Object with a special TypeScript type
template <typename T> class JSIteratorResult : public Napi::Object {
public:
  JSIteratorResult(const Napi::Object &obj) : Napi::Object(obj) {}
};

// The common base of the copying and the reference iterators
template <typename T, const ReturnAttribute &RETATTR> class JSIterator {
  T &target;
  typename T::iterator it;
  // Ideally this should be an optional value
  // but this requires C++20
  Napi::Reference<Napi::Value> *persistent;
  using value_type_t = typename T::iterator::value_type;

public:
  using type = JSIterator<T, RETATTR>;
  JSIterator(T &obj, const typename T::iterator &begin, Napi::Value jsobj)
      : target(obj), it(begin), persistent(nullptr) {
    if constexpr (RETATTR.isNested()) {
      persistent = new Napi::Reference<Napi::Value>;
      *persistent = Napi::Persistent(jsobj);
    }
  }
  JSIterator(JSIterator<T, RETATTR> &) = delete;
  virtual ~JSIterator() {
    if constexpr (RETATTR.isNested()) {
      delete persistent;
    }
  };
  Nobind::JSIteratorResult<value_type_t> next(Napi::Env env) {
    static_assert(!RETATTR.isCopy() || std::is_copy_constructible_v<value_type_t>,
                  "JSIterator<ReturnCopy> works only with copy-constructible objects");
    static_assert(!RETATTR.isNested() || !std::is_scalar_v<value_type_t>,
                  "JSIterator<ReturnNested> should not be used with scalar values");

    JSIteratorResult<value_type_t> ret = Napi::Object::New(env);
    if (this->it == this->target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      Napi::Value value;
      if constexpr (RETATTR.isCopy()) {
        value = Nobind::ToJS<value_type_t, Nobind::ReturnCopy>(env, *it).Get();
      }
      if constexpr (RETATTR.isNested()) {
        value = Nobind::ToJS<value_type_t &, Nobind::ReturnShared>(env, *it).Get();
        // Every returned object receives a copy of the reference to his container
        // Thus, the container can be destroyed only after all returned objects and
        // the iterator have been GCed
        value.ToObject().DefineProperty(
            Napi::PropertyDescriptor::Value(Napi::String::New(env, NOBIND_PARENT_PROP), persistent->Value()));
      }
      ret.Set("value", value);
      ret.Set("done", Napi::Boolean::New(env, false));
      it++;
    }
    return ret;
  }
};

// Helper to construct the iterators
template <typename T, const ReturnAttribute &RETATTR> JSIterator<T, RETATTR> *MakeJSIterator(Napi::Value jsobj) {
  static_assert(RETATTR.isCopy() || RETATTR.isNested(),
                "JSMakeIterator supports only explicit ReturnNested or ReturnCopy, "
                "refer to the documentation to see why");
  T &obj = FromJSValue<T &>(jsobj).Get();
  return new JSIterator<T, RETATTR>{obj, obj.begin(), jsobj};
};

} // namespace Nobind
