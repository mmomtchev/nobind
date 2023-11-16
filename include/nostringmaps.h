#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

template <typename T> class FromJSString {
  std::remove_cv_t<std::remove_reference_t<T>> val_;

public:
  inline explicit FromJSString(const Napi::Value &val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    val_ = val.ToString().Utf8Value();
  }
  inline T Get() { return val_; }
  FromJSString(const FromJSString &) = delete;
  FromJSString(FromJSString &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSString {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSString(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::String::New(env_, val_); }
  ToJSString(const ToJSString &) = delete;
  ToJSString(ToJSString &&) = default;
};

template <typename T> class FromJSChar {
  char *val_;

public:
  inline explicit FromJSChar(const Napi::Value &val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    std::string s = val.ToString().Utf8Value();
    val_ = new char[s.size() + 1];
    strncpy(val_, s.c_str(), s.size());
    val_[s.size()] = 0;
  }

  inline T Get() { return val_; }

  ~FromJSChar() { delete val_; }
  FromJSChar(const FromJSChar &) = delete;
  FromJSChar(FromJSChar &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSChar {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSChar(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::String::New(env_, val_); }
  ToJSChar(const ToJSChar &) = delete;
  ToJSChar(ToJSChar &&) = default;
};

#define TYPEMAPS_FOR_STRING(TYPE, CLASS)                                                                               \
  template <> struct FromJS<TYPE> : public FromJS##CLASS<TYPE> {                                                       \
    using FromJS##CLASS<TYPE>::FromJS##CLASS;                                                                          \
  };                                                                                                                   \
  template <const ReturnAttribute &RETATTR> struct ToJS<TYPE, RETATTR> : public ToJS##CLASS<TYPE, RETATTR> {           \
    using ToJS##CLASS<TYPE, RETATTR>::ToJS##CLASS;                                                                     \
  };

// The const versions are needed to ensure that we
// do not end up using the T& specialization (ie we are always more specialized)
TYPEMAPS_FOR_STRING(std::string, String);
TYPEMAPS_FOR_STRING(const std::string, String);
TYPEMAPS_FOR_STRING(std::string &, String);
TYPEMAPS_FOR_STRING(const std::string &, String);

// remove_const_t<const char*> == const char* (it is a pointer before being a const)
// remove_const_t<char const*> == char* (but few people use this notation)
// remove_pointer_t<const char*> == const char
// remove_const_t<remove_pointer_t<const char *>> == char
TYPEMAPS_FOR_STRING(char *, Char);
TYPEMAPS_FOR_STRING(const char *, Char);

} // namespace Typemap

} // namespace Nobind
