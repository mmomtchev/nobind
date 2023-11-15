#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

#define TYPEMAPS_FOR_STD_STRING(TYPE)                                                                                  \
  template <> class FromJS<TYPE> {                                                                                     \
    std::remove_cv_t<std::remove_reference_t<TYPE>> val_;                                                              \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(const Napi::Value &val) {                                                                   \
      if (!val.IsString()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Expected a string");                                                         \
      }                                                                                                                \
      val_ = val.ToString().Utf8Value();                                                                               \
    }                                                                                                                  \
    inline TYPE Get() { return val_; }                                                                                 \
  };                                                                                                                   \
                                                                                                                       \
  template <const ReturnAttribute &RETATTR> class ToJS<TYPE, RETATTR> {                                                \
    Napi::Env env_;                                                                                                    \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, TYPE val) : env_(env), val_(val) {}                                            \
    inline Napi::Value Get() { return Napi::String::New(env_, val_); }                                                 \
  };

// The const versions are needed to ensure that we
// do not end up using the T& specialization (ie we are always more specialized)
TYPEMAPS_FOR_STD_STRING(std::string);
TYPEMAPS_FOR_STD_STRING(const std::string);
TYPEMAPS_FOR_STD_STRING(std::string &);
TYPEMAPS_FOR_STD_STRING(const std::string &);

#define TYPEMAPS_FOR_CHAR_STRING(TYPE)                                                                                 \
  template <> class FromJS<TYPE> {                                                                                     \
    char *val_;                                                                                                        \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(const Napi::Value &val) {                                                                   \
      if (!val.IsString()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Expected a string");                                                         \
      }                                                                                                                \
      std::string s = val.ToString().Utf8Value();                                                                      \
      val_ = new char[s.size() + 1];                                                                                   \
      strncpy(val_, s.c_str(), s.size());                                                                              \
      val_[s.size()] = 0;                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    inline TYPE Get() { return val_; }                                                                                 \
                                                                                                                       \
    ~FromJS() { delete val_; }                                                                                         \
  };                                                                                                                   \
                                                                                                                       \
  template <const ReturnAttribute &RETATTR> class ToJS<TYPE, RETATTR> {                                                \
    Napi::Env env_;                                                                                                    \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, TYPE val) : env_(env), val_(val) {}                                            \
    inline Napi::Value Get() { return Napi::String::New(env_, val_); }                                                 \
  };

// Same explanation as above:
// remove_const_t<const char*> == const char* (it is a pointer before being a const)
// remove_const_t<char const*> == char* (but few people use this notation)
// remove_pointer_t<const char*> == const char
// remove_const_t<remove_pointer_t<const char *>> == char
TYPEMAPS_FOR_CHAR_STRING(char *);
TYPEMAPS_FOR_CHAR_STRING(const char *);

} // namespace Typemap

} // namespace Nobind
