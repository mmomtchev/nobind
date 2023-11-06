#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

#define TYPEMAPS_FOR_INT32(TYPE)                                                                                       \
  template <> class FromJS<TYPE> {                                                                                     \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsNumber()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a number");                                                         \
      }                                                                                                                \
      val_ = static_cast<TYPE>(val.ToNumber().Int32Value());                                                           \
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
    inline Napi::Value operator*() { return Napi::Number::New(env_, static_cast<int32_t>(val_)); }                     \
  };

#define TYPEMAPS_FOR_UINT32(TYPE)                                                                                      \
  template <> class FromJS<TYPE> {                                                                                     \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsNumber()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a number");                                                         \
      }                                                                                                                \
      val_ = static_cast<TYPE>(val.ToNumber().Uint32Value());                                                          \
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
    inline Napi::Value operator*() { return Napi::Number::New(env_, static_cast<uint32_t>(val_)); }                    \
  };

#define TYPEMAPS_FOR_INT64(TYPE)                                                                                       \
  template <> class FromJS<TYPE> {                                                                                     \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsNumber()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a number");                                                         \
      }                                                                                                                \
      val_ = static_cast<TYPE>(val.ToNumber().Int64Value());                                                           \
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
    inline Napi::Value operator*() { return Napi::Number::New(env_, static_cast<int64_t>(val_)); }                     \
  };

#define TYPEMAPS_FOR_DOUBLE(TYPE)                                                                                      \
  template <> class FromJS<TYPE> {                                                                                     \
    TYPE val_;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsNumber()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not a number");                                                         \
      }                                                                                                                \
      val_ = static_cast<TYPE>(val.ToNumber().DoubleValue());                                                          \
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
    inline Napi::Value operator*() { return Napi::Number::New(env_, static_cast<double>(val_)); }                      \
  };

TYPEMAPS_FOR_INT32(int);
TYPEMAPS_FOR_INT32(short);
TYPEMAPS_FOR_INT32(long);
TYPEMAPS_FOR_UINT32(unsigned);
TYPEMAPS_FOR_UINT32(unsigned short);
TYPEMAPS_FOR_UINT32(unsigned long);
TYPEMAPS_FOR_INT64(long long);
TYPEMAPS_FOR_INT64(unsigned long long);
TYPEMAPS_FOR_DOUBLE(double);
TYPEMAPS_FOR_DOUBLE(float);

} // namespace Typemap

} // namespace Nobind
