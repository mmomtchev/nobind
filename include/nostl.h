#pragma once
#include <map>
#include <notypes.h>
#include <string>
#include <vector>

namespace Nobind {

namespace Typemap {

template <typename V, typename T> class FromJSVector {
  std::remove_cv_t<std::remove_reference_t<V>> val_;

public:
  inline explicit FromJSVector(const Napi::Value &val) {
    if (!val.IsArray()) {
      throw Napi::TypeError::New(val.Env(), "Expected an array");
    }
    Napi::Array array = val.As<Napi::Array>();
    val_.reserve(array.Length());
    for (size_t i = 0; i < array.Length(); i++) {
      val_.push_back(FromJSValue<T>(array.Get(i)).Get());
    }
  }

  inline V Get() { return val_; }
  FromJSVector(const FromJSVector &) = delete;
  FromJSVector(FromJSVector &&) = default;
};

template <typename V, typename T, const ReturnAttribute &RETATTR> class ToJSVector {
  Napi::Env env_;
  std::remove_cv_t<std::remove_reference_t<V>> val_;

public:
  inline explicit ToJSVector(Napi::Env env, V val) : env_(env), val_(val) {}
  inline Napi::Value Get() {
    Napi::Array array = Napi::Array::New(env_, val_.size());
    for (size_t i = 0; i < val_.size(); i++) {
      array.Set(i, ToJS<T, RETATTR>(env_, val_[i]).Get());
    }
    return array;
  }
  ToJSVector(const ToJSVector &) = delete;
  ToJSVector(ToJSVector &&) = default;
};

template <typename M, typename T> class FromJSMap {
  std::remove_cv_t<std::remove_reference_t<M>> val_;

public:
  inline explicit FromJSMap(const Napi::Value &val) {
    if (!val.IsObject()) {
      throw Napi::TypeError::New(val.Env(), "Expected an object");
    }
    Napi::Object object = val.ToObject();
    for (auto prop : object) {
      val_.insert({prop.first.ToString().Utf8Value(), FromJSValue<T>(prop.second).Get()});
    }
  }

  inline M Get() { return val_; }
  FromJSMap(const FromJSMap &) = delete;
  FromJSMap(FromJSMap &&) = default;
};

template <typename M, typename T, const ReturnAttribute &RETATTR> class ToJSMap {
  Napi::Env env_;
  std::remove_cv_t<std::remove_reference_t<M>> val_;

public:
  inline explicit ToJSMap(Napi::Env env, M val) : env_(env), val_(val) {}
  inline Napi::Value Get() {
    Napi::Object object = Napi::Object::New(env_);
    for (auto const &prop : val_) {
      object.Set(Napi::String::New(env_, prop.first), ToJS<T, RETATTR>(env_, prop.second).Get());
    }
    return object;
  }
  ToJSMap(const ToJSMap &) = delete;
  ToJSMap(ToJSMap &&) = default;
};

#define TYPEMAPS_FOR_STL(CONTAINER, CLASS)                                                                             \
  template <typename T> struct FromJS<CONTAINER> : public FromJS##CLASS<CONTAINER, T> {                                \
    using FromJS##CLASS<CONTAINER, T>::FromJS##CLASS;                                                                  \
  };                                                                                                                   \
  template <typename T, const ReturnAttribute &RETATTR>                                                                \
  struct ToJS<CONTAINER, RETATTR> : public ToJS##CLASS<CONTAINER, T, RETATTR> {                                        \
    using ToJS##CLASS<CONTAINER, T, RETATTR>::ToJS##CLASS;                                                             \
  };

TYPEMAPS_FOR_STL(std::vector<T>, Vector);
TYPEMAPS_FOR_STL(std::vector<T> &, Vector);
TYPEMAPS_FOR_STL(const std::vector<T>, Vector);
TYPEMAPS_FOR_STL(const std::vector<T> &, Vector);

template <typename T> using string_map = std::map<std::string, T>;
TYPEMAPS_FOR_STL(string_map<T>, Map);
TYPEMAPS_FOR_STL(string_map<T> &, Map);
TYPEMAPS_FOR_STL(const string_map<T>, Map);
TYPEMAPS_FOR_STL(const string_map<T> &, Map);

} // namespace Typemap

} // namespace Nobind
