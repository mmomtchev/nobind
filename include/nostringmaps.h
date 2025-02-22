#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

template <typename T> class FromJSString {
  std::remove_cv_t<std::remove_reference_t<T>> val_;

public:
  NOBIND_INLINE explicit FromJSString(const Napi::Value &val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    val_ = val.ToString().Utf8Value();
  }
  NOBIND_INLINE T Get() { return val_; }
  FromJSString(const FromJSString &) = delete;
  FromJSString(FromJSString &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSString {
  Napi::Env env_;
  T val_;

public:
  NOBIND_INLINE explicit ToJSString(Napi::Env env, T val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() { return Napi::String::New(env_, val_); }
  ToJSString(const ToJSString &) = delete;
  ToJSString(ToJSString &&) = default;
};

template <typename T> class FromJSChar {
  char *val_;

public:
  NOBIND_INLINE explicit FromJSChar(const Napi::Value &val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    std::string s = val.ToString().Utf8Value();
    val_ = new char[s.size() + 1];
    strncpy(val_, s.c_str(), s.size());
    val_[s.size()] = 0;
  }

  NOBIND_INLINE T Get() { return val_; }

  ~FromJSChar() { delete val_; }
  FromJSChar(const FromJSChar &) = delete;
  FromJSChar(FromJSChar &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSChar {
  Napi::Env env_;
  T val_;

public:
  NOBIND_INLINE explicit ToJSChar(Napi::Env env, T val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() { return Napi::String::New(env_, val_); }
  ToJSChar(const ToJSChar &) = delete;
  ToJSChar(ToJSChar &&) = default;
};

const std::string string_tstype = "string"s;

#define TYPEMAPS_FOR_STRING(TYPE, CLASS)                                                                               \
  template <> struct FromJS<TYPE> : public FromJS##CLASS<TYPE> {                                                       \
    using FromJS##CLASS<TYPE>::FromJS##CLASS;                                                                          \
    static const std::string &TSType() { return string_tstype; };                                                      \
  };                                                                                                                   \
  template <const ReturnAttribute &RETATTR> struct ToJS<TYPE, RETATTR> : public ToJS##CLASS<TYPE, RETATTR> {           \
    using ToJS##CLASS<TYPE, RETATTR>::ToJS##CLASS;                                                                     \
    static const std::string &TSType() { return string_tstype; };                                                      \
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
