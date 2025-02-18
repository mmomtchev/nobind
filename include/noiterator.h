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

template <typename T> class JSIterator {
protected:
  T &target;
  typename T::iterator it;
  using value_type_t = typename T::iterator::value_type;

public:
  JSIterator(T &obj, const typename T::iterator &begin) : target(obj), it(begin) {}
  virtual ~JSIterator() = default;
  virtual Nobind::JSIteratorResult<value_type_t> next(Napi::Env env) = 0;
};

// This is the copying iterator
template <typename T> class JSCopyIterator : public JSIterator<T> {
  using value_type_t = typename JSIterator<T>::value_type_t;

public:
  JSCopyIterator(T &obj, const typename T::iterator &begin) : JSIterator<T>{obj, begin} {}
  virtual JSIteratorResult<value_type_t> next(Napi::Env env) override {
    static_assert(std::is_copy_constructible_v<value_type_t>,
                  "JSCopyIterator works only with copy-constructible objects");

    JSIteratorResult<value_type_t> ret = Napi::Object::New(env);
    if (this->it == this->target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      ret.Set("value", Nobind::ToJS<value_type_t, Nobind::ReturnOwned>(env, *this->it).Get());
      ret.Set("done", Napi::Boolean::New(env, false));
      this->it++;
    }
    return ret;
  }
};

// This is the shared reference iterator
template <typename T> class JSReferenceIterator : public JSIterator<T> {
  using value_type_t = typename JSIterator<T>::value_type_t;

  // The iterator keeps a JS reference to his container to protect it
  // from the GC
  Napi::Reference<Napi::Value> persistent;

public:
  JSReferenceIterator(T &) = delete;
  JSReferenceIterator(T &obj, const typename T::iterator &begin, Napi::Value jsobj)
      : JSIterator<T>{obj, begin}, persistent(Napi::Persistent(jsobj)) {}
  JSIteratorResult<value_type_t> next(Napi::Env env) {
    static_assert(!std::is_scalar_v<value_type_t>, "JSReferenceIterator should not be used with scalar values");

    JSIteratorResult<value_type_t> ret = Napi::Object::New(env);
    if (this->it == this->target.end()) {
      ret.Set("done", Napi::Boolean::New(env, true));
    } else {
      Napi::Value value = Nobind::ToJS<value_type_t &, Nobind::ReturnShared>(env, *this->it).Get();
      // Every returned object receives a copy of the reference to his container
      // Thus, the container can be destroyed only after all returned objects and
      // the iterator have been GCed
      value.ToObject().DefineProperty(
          Napi::PropertyDescriptor::Value(Napi::String::New(env, NOBIND_PARENT_PROP), persistent.Value()));
      ret.Set("value", value);
      ret.Set("done", Napi::Boolean::New(env, false));
      this->it++;
    }
    return ret;
  }
};

// Helpers to construct the iterators
// The safe iterator does not need anything
template <typename T> JSCopyIterator<T> MakeJSCopyIterator(T &obj) {
  return Nobind::JSCopyIterator<T>{obj, obj.begin()};
}

// The "dangerous" iterator keeps a JS reference to the container to protect it from the GC
// This iterator is not copy-constructible and must be allocated on the heap
// (Nobind will recognize the pointer type and it will take care of the destruction
// because with pointers, Nobind::ReturnDefault considers it a dynamically allocated object)
template <typename T> JSReferenceIterator<T> *MakeJSReferenceIterator(Napi::Value jsobj) {
  // Unwrap the JS value to get the C++ object
  T &obj = Nobind::FromJSValue<T &>(jsobj).Get();
  return new Nobind::JSReferenceIterator<T>{obj, obj.begin(), jsobj};
}

} // namespace Nobind
