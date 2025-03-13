#pragma once
#include <nonapi.h>
#include <string>
#include <tuple>
#include <type_traits>

#ifndef NOBIND_PARENT_PROP
#define NOBIND_PARENT_PROP "__nobind_parent_reference"
#endif

using namespace std::literals::string_literals;

#include <noattributes.h>
#include <nooverrides.h>

namespace Nobind {

inline void CheckArgLength(Napi::Env env, size_t expected, size_t actual) {
  if (actual != expected) {
    std::string msg = "Expected "s + std::to_string(expected) + " arguments, got "s + std::to_string(actual);
    throw Napi::TypeError::New(env, msg);
  }
}

const std::string boolean_tstype = "boolean"s;

// https://stackoverflow.com/questions/22825512/get-type-of-member-memberpointer-points-to
template <class C, typename T> T getMemberPointerType(T C::*v);

namespace Typemap {

// bool specializations
template <const ReturnAttribute &RETATTR> class ToJS<bool, RETATTR> {
  Napi::Env env_;
  bool val_;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, bool val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() { return Napi::Boolean::New(env_, val_); }

  static const std::string &TSType() { return boolean_tstype; };
};

template <> class FromJS<bool> {
  bool val_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    if (!val.IsBoolean()) {
      throw Napi::TypeError::New(val.Env(), "Expected a boolean");
    }
    val_ = val.ToBoolean().Value();
  }
  NOBIND_INLINE bool Get() { return val_; }

  static const std::string &TSType() { return boolean_tstype; };
};

// native specializations (does not support async)
template <const ReturnAttribute &RETATTR> class ToJS<Napi::Value, RETATTR> {
  Napi::Env env_;
  Napi::Value val_;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, Napi::Value val) : env_(env), val_(val) {
    static_assert(!RETATTR.isAsync(), "The native typemaps are not compatible with async mode");
  }
  NOBIND_INLINE Napi::Value Get() { return val_; }
};

template <> class FromJS<Napi::Value> {
  Napi::Value val_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) : val_(val) {}
  NOBIND_INLINE Napi::Value Get() { return val_; }
};

// Typemap that generates Napi::Env arguments w/o consuming input
template <> class FromJS<Napi::Env> {
  Napi::Env val_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) : val_(val.Env()) {}
  NOBIND_INLINE const Napi::Env Get() { return val_; }
  static const std::string TSType() { return ""; };

  static const size_t Inputs = 0;
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

// Standard C++ detection idiom (SFINAE version)
// adapted from https://benjaminbrock.net/blog/detection_idiom.php

// Detects if the Typemap has declared Inputs
template <typename T> class FromJSTypemapHasInputs {
  template <typename U> static constexpr decltype(std::declval<U &>().Inputs, bool()) test(int) { return true; }
  template <typename U> static constexpr NOBIND_INLINE bool test(...) { return false; }

public:
  static constexpr bool value = test<T>(int());
};

// Detects if the Typemap has Lock() and Unlock()
template <typename T> class FromJSTypemapHasLocking {
  template <typename U> static constexpr decltype(std::declval<U &>().Lock(), bool()) test_Lock(int) { return true; }
  template <typename U> static constexpr NOBIND_INLINE bool test_Lock(...) { return false; }

  template <typename U> static constexpr decltype(std::declval<U &>().Unlock(), bool()) test_Unlock(int) {
    return true;
  }
  template <typename U> static constexpr NOBIND_INLINE bool test_Unlock(...) { return false; }

public:
  static constexpr bool lock = test_Lock<T>(int());
  static constexpr bool unlock = test_Unlock<T>(int());
};

// Main entry point when processing a Napi::Value
template <typename T> auto NOBIND_INLINE FromJSValue(const Napi::Value &val) {
  if constexpr (std::is_constructible_v<TypemapOverrides::FromJS<std::remove_cv_t<T>>, const Napi::Value &>) {
    return TypemapOverrides::FromJS<std::remove_cv_t<T>>(val);
  } else {
    return Typemap::FromJS<std::remove_cv_t<T>>(val);
  }
}

// Type getter for the above method
template <typename T>
using FromJS_t = typename std::invoke_result_t<decltype(Nobind::FromJSValue<std::remove_cv_t<T>>), const Napi::Value &>;

// Main entry point when processing a value from arguments
template <typename T> auto NOBIND_INLINE FromJSArgs(const Napi::CallbackInfo &info, size_t &idx) {
  // Get the template specialization that will be used from the result of FromJSArgs for T
  using FromJSTypeMap = FromJS_t<std::remove_cv_t<T>>;

  size_t current_idx = idx;

  // Check if this template specialization has Inputs
  if constexpr (FromJSTypemapHasInputs<FromJSTypeMap>::value) {
    static_assert(FromJSTypeMap::Inputs == 0 || FromJSTypeMap::Inputs == 1,
                  "Only 1:1 and 1:0 conversions are supported at the moment");
    idx += FromJSTypeMap::Inputs;
  } else {
    idx++;
  }
  // Construct in place to avoid copying
  // (this type is potentially not copy-constructible)
  return FromJSValue<std::remove_cv_t<T>>(info[current_idx]);
}

// Main entry point when generating a Napi::Value
template <typename T, const ReturnAttribute &RETATTR> auto NOBIND_INLINE ToJS(const Napi::Env &env, T val) {
  if constexpr (std::is_constructible_v<TypemapOverrides::ToJS<std::remove_cv_t<T>, RETATTR>, const Napi::Env &, T>) {
    return TypemapOverrides::ToJS<std::remove_cv_t<T>, RETATTR>(env, val);
  } else if constexpr (std::is_constructible_v<TypemapOverrides::ToJS<std::remove_cv_t<T>>, const Napi::Env &, T>) {
    return TypemapOverrides::ToJS<std::remove_cv_t<T>>(env, val);
  } else {
    return Typemap::ToJS<std::remove_cv_t<T>, RETATTR>(env, val);
  }
}

// Type getter for the above method
template <typename T, const ReturnAttribute &RETATTR>
using ToJS_t =
    typename std::invoke_result_t<decltype(Nobind::ToJS<never_void_t<T>, RETATTR>), const Napi::Env &, never_void_t<T>>;

#ifndef NOBIND_NO_ASYNC_LOCKING
// A RAII guard that calls FromJS::Lock()/Unlock() if the typemap has them
template <typename T> class FromJSLockGuard {
  FromJS_t<T> &tm_;

public:
  FromJSLockGuard(FromJS_t<T> &tm) : tm_(tm) {
    if constexpr (FromJSTypemapHasLocking<FromJS_t<T>>::lock) {
      tm_.Lock();
    }
  };
  virtual ~FromJSLockGuard() {
    if constexpr (FromJSTypemapHasLocking<FromJS_t<T>>::unlock) {
      tm_.Unlock();
    }
  }

  FromJSLockGuard(const FromJSLockGuard &) = delete;
};
#endif

} // namespace Nobind
