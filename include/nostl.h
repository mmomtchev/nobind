#pragma once
#include <map>
#include <notypes.h>
#include <notypescript.h>
#include <string>
#include <vector>

namespace Nobind {

namespace Typemap {

template <typename V, typename T> class FromJSVector {
  std::remove_cv_t<std::remove_reference_t<V>> val_;

public:
  NOBIND_INLINE explicit FromJSVector(const Napi::Value &val) {
    if (!val.IsArray()) {
      throw Napi::TypeError::New(val.Env(), "Expected an array");
    }
    Napi::Array array = val.As<Napi::Array>();
    val_.reserve(array.Length());
    for (size_t i = 0; i < array.Length(); i++) {
      auto val = FromJSValue<T>(array.Get(i));
#ifndef NOBIND_NO_ASYNC_LOCKING
      FromJSLockGuard<T> guard{val};
#endif
      val_.push_back(val.Get());
    }
  }

  NOBIND_INLINE V Get() { return val_; }
  FromJSVector(const FromJSVector &) = delete;
  FromJSVector(FromJSVector &&) = default;

  static std::string TSType() { return createTSArray<T>(); };
};

template <typename V, typename T, const ReturnAttribute &RETATTR> class ToJSVector {
  Napi::Env env_;
  std::remove_cv_t<std::remove_reference_t<V>> val_;

public:
  NOBIND_INLINE explicit ToJSVector(Napi::Env env, V val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() {
    Napi::Array array = Napi::Array::New(env_, val_.size());
    for (size_t i = 0; i < val_.size(); i++) {
      array.Set(i, ToJS<T, RETATTR>(env_, val_[i]).Get());
    }
    return array;
  }
  ToJSVector(const ToJSVector &) = delete;
  ToJSVector(ToJSVector &&) = default;

  static std::string TSType() { return createTSArray<T>(); };
};

template <typename M, typename T> class FromJSMap {
  std::remove_cv_t<std::remove_reference_t<M>> val_;

public:
  NOBIND_INLINE explicit FromJSMap(const Napi::Value &val) {
    if (!val.IsObject()) {
      throw Napi::TypeError::New(val.Env(), "Expected an object");
    }
    Napi::Object object = val.ToObject();
    for (auto prop : object) {
      auto tm = FromJSValue<T>(prop.second);
#ifndef NOBIND_NO_ASYNC_LOCKING
      FromJSLockGuard<T> guard{tm};
#endif
      val_.insert({prop.first.ToString().Utf8Value(), tm.Get()});
    }
  }

  NOBIND_INLINE M Get() { return val_; }
  FromJSMap(const FromJSMap &) = delete;
  FromJSMap(FromJSMap &&) = default;

  static std::string TSType() { return createTSRecord<std::string, T>(); };
};

template <typename M, typename T, const ReturnAttribute &RETATTR> class ToJSMap {
  Napi::Env env_;
  std::remove_cv_t<std::remove_reference_t<M>> val_;

public:
  NOBIND_INLINE explicit ToJSMap(Napi::Env env, M val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() {
    Napi::Object object = Napi::Object::New(env_);
    for (auto &prop : val_) {
      object.Set(Napi::String::New(env_, prop.first), ToJS<T, RETATTR>(env_, prop.second).Get());
    }
    return object;
  }
  ToJSMap(const ToJSMap &) = delete;
  ToJSMap(ToJSMap &&) = default;

  static std::string TSType() { return createTSRecord<std::string, T>(); };
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
