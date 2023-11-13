#pragma once
#include <napi.h>
#include <string>
#include <tuple>
#include <type_traits>

using namespace std::literals::string_literals;

#include <noattributes.h>
#include <nooverrides.h>

namespace Nobind {

template <typename... ARGS> inline void CheckArgLength(Napi::Env env, size_t len) {
  if (len != sizeof...(ARGS)) {
    // constexpr strings are C++20
    static const std::string msg = "Expected "s + std::to_string(sizeof...(ARGS)) + " arguments, got "s;
    throw Napi::TypeError::New(env, msg + std::to_string(len));
  }
}

// https://stackoverflow.com/questions/22825512/get-type-of-member-memberpointer-points-to
template <class C, typename T> T getMemberPointerType(T C::*v);

namespace Typemap {

// bool specializations
template <const ReturnAttribute &RETATTR> class ToJS<bool, RETATTR> {
  Napi::Env env_;
  bool val_;

public:
  inline explicit ToJS(Napi::Env env, bool val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Boolean::New(env_, val_); }
};

template <> class FromJS<bool> {
  bool val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if (!val.IsBoolean()) {
      throw Napi::TypeError::New(val.Env(), "Not a boolean");
    }
    val_ = val.ToBoolean().Value();
  }
  inline bool Get() { return val_; }
};

// native specializations (does not support async)
template <const ReturnAttribute &RETATTR> class ToJS<Napi::Value, RETATTR> {
  Napi::Env env_;
  Napi::Value val_;

public:
  inline explicit ToJS(Napi::Env env, Napi::Value val) : env_(env), val_(val) {
    static_assert(!RETATTR.isAsync(), "The native typemaps are not compatible with async mode");
  }
  inline Napi::Value Get() { return val_; }
};

template <> class FromJS<Napi::Value> {
  Napi::Value val_;

public:
  inline explicit FromJS(const Napi::Value &val) : val_(val) {}
  inline Napi::Value Get() { return val_; }
};

} // namespace Typemap

// never_void is a helper that allows to declare function arguments
// that can potentially be a void type
template <typename T> struct never_void {
  typedef T type;
};
template <> struct never_void<void> {
  typedef int type;
};
template <typename T> using never_void_t = typename never_void<T>::type;

// Main entry point when processing a Napi::Value, should return a prvalue to a Typemap::FromJS
template <typename T> auto inline FromJS(const Napi::Value &val) {
  if constexpr (std::is_constructible_v<TypemapOverrides::FromJS<std::remove_cv_t<T>>, const Napi::Value &>) {
    return TypemapOverrides::FromJS<std::remove_cv_t<T>>(val);
  } else {
    return Typemap::FromJS<std::remove_cv_t<T>>(val);
  }
}

// Main entry point when generating a Napi::Value, should return a prvalue to a Typemap::ToJS
template <typename T, const ReturnAttribute &RETATTR> auto inline ToJS(const Napi::Env &env, T val) {
  if constexpr (std::is_constructible_v<TypemapOverrides::ToJS<std::remove_cv_t<T>, RETATTR>, const Napi::Env &, T>) {
    return TypemapOverrides::ToJS<std::remove_cv_t<T>, RETATTR>(env, val);
  } else if constexpr (std::is_constructible_v<TypemapOverrides::ToJS<std::remove_cv_t<T>>, const Napi::Env &, T>) {
    return TypemapOverrides::ToJS<std::remove_cv_t<T>>(env, val);
  } else {
    return Typemap::ToJS<std::remove_cv_t<T>, RETATTR>(env, val);
  }
}

// Type getters for the above methods
// These has been specially crafted to avoid triggering a compiler crash in g++-11 (fixed in g++-12)
template <typename T> using FromJS_t = typename std::invoke_result_t<decltype(Nobind::FromJS<T>), const Napi::Value &>;

template <typename T, const ReturnAttribute &RETATTR>
using ToJS_t =
    typename std::invoke_result_t<decltype(Nobind::ToJS<never_void_t<T>, RETATTR>), const Napi::Env &, never_void_t<T>>;

} // namespace Nobind
