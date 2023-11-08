#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

#define TYPEMAPS_FOR_STD_STRING(TYPE)                                                                                  \
  template <> class FromJS<TYPE> {                                                                                     \
    std::remove_cv_t<std::remove_reference_t<TYPE>> val_;                                                              \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsString()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a string");                                                         \
      }                                                                                                                \
      val_ = val.ToString().Utf8Value();                                                                               \
    }                                                                                                                  \
    inline TYPE operator*() { return val_; }                                                                           \
  };                                                                                                                   \
                                                                                                                       \
  template <const ReturnAttribute &RETATTR> class ToJS<TYPE, RETATTR> {                                                \
    Napi::Env env_;                                                                                                    \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, TYPE val) : env_(env), val_(val) {}                                            \
    inline Napi::Value operator*() { return Napi::String::New(env_, val_); }                                           \
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
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsString()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a string");                                                         \
      }                                                                                                                \
      std::string s = val.ToString().Utf8Value();                                                                      \
      val_ = new char[s.size() + 1];                                                                                   \
      strncpy(val_, s.c_str(), s.size());                                                                              \
      val_[s.size()] = 0;                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    inline TYPE operator*() { return val_; }                                                                           \
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
    inline Napi::Value operator*() { return Napi::String::New(env_, val_); }                                           \
  };

TYPEMAPS_FOR_CHAR_STRING(char *);
TYPEMAPS_FOR_CHAR_STRING(const char *);

} // namespace Typemap

} // namespace Nobind
