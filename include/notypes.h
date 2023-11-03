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

namespace Typemap {

// Typemap::FromJS rules
// - The constructor should check the incoming value
// - operator* should return an in-place constructed prvalue
// - operator* can be called in a background thread - no V8 Local<>s allowed
// - The constructor can create state that will be destroyed after the function call
template <typename T> class FromJS {
public:
  inline FromJS(Napi::Value val) { static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap"); }
  inline T operator*() { return T(); }
};

template <typename T> class ToJS {
public:
  inline ToJS(Napi::Env env, T val) { static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap"); }
  inline Napi::Value operator*() { return Napi::Value(); }
};

template <> class ToJS<bool> {
  Napi::Env env_;
  bool val_;

public:
  inline ToJS(Napi::Env env, bool val) : env_(env), val_(val) {}
  inline Napi::Value operator*() { return Napi::Boolean::New(env_, val_); }
};

template <> class FromJS<bool> {
  bool val_;

public:
  inline FromJS(Napi::Value val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Not a number");
    }
    val_ = val.ToBoolean().Value();
  }
  inline bool operator*() { return val_; }
};


} // namespace Typemap

// Main entry point when processing a Napi::Value, should return a prvalue to a Typemap::FromJS
template <typename T> auto inline FromJS(const Napi::Value &val) { return Typemap::FromJS<std::remove_cv_t<T>>(val); }

} // namespace Nobind
