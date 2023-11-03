#pragma once
#include <napi.h>
#include <string>
#include <tuple>
#include <type_traits>
using namespace std::literals::string_literals;

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

template <typename T> class Typemap {
public:
  static inline T FromJS(Napi::Value val) {
    static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    return T();
  }
  static inline Napi::Value ToJS(Napi::Env env, T val) {
    static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    return Napi::Value();
  }
};

template <> class Typemap<int> {
public:
  static inline int FromJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToNumber().Int32Value();
  }
  static inline Napi::Value ToJS(Napi::Env env, int val) { return Napi::Number::New(env, val); }
};

template <> class Typemap<double> {
public:
  static inline double FromJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToNumber().DoubleValue();
  }
  static inline Napi::Value ToJS(Napi::Env env, double val) { return Napi::Number::New(env, val); }
};

template <> class Typemap<bool> {
public:
  static inline bool FromJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToBoolean().Value();
  }
  static inline Napi::Value ToJS(Napi::Env env, bool val) { return Napi::Boolean::New(env, val); }
};

template <> class Typemap<std::string> {
public:
  static inline std::string FromJS(Napi::Value val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Not a string");
    }
    return val.ToString().Utf8Value();
  }
  static inline Napi::Value ToJS(Napi::Env env, std::string val) { return Napi::String::New(env, val); }
};

// Main entry point when processing a Napi::Value
template <typename T> T inline FromJS(const Napi::Value &val) {
  return Typemap<T>::FromJS(val);
}

} // namespace Nobind
