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

template <typename T> class Typemap {
public:
  static inline T ToJS(Napi::Value val) {
    static_assert(std::is_same<T, T>(), "Type does not have a ToJS typemap");
    return T();
  }
  static inline Napi::Value FromJS(Napi::Env env, T val) {
    static_assert(std::is_same<T, T>(), "Type does not have a FromJS typemap");
    return Napi::Value();
  }
};

template <> class Typemap<int> {
public:
  static inline int ToJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToNumber().Int32Value();
  }
  static inline Napi::Value FromJS(Napi::Env env, int val) { return Napi::Number::New(env, val); }
};

template <> class Typemap<double> {
public:
  static inline double ToJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToNumber().DoubleValue();
  }
  static inline Napi::Value FromJS(Napi::Env env, double val) { return Napi::Number::New(env, val); }
};

template <> class Typemap<bool> {
public:
  static inline bool ToJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    return val.ToBoolean().Value();
  }
  static inline Napi::Value FromJS(Napi::Env env, bool val) { return Napi::Boolean::New(env, val); }
};

template <> class Typemap<std::string> {
public:
  static inline std::string ToJS(Napi::Value val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Not a string");
    }
    return val.ToString().Utf8Value();
  }
  static inline Napi::Value FromJS(Napi::Env env, std::string val) { return Napi::String::New(env, val); }
};

} // namespace Nobind
