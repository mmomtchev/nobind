#pragma once
#include <notypes.h>

namespace Nobind {

namespace Typemap {

template <typename T> class FromJSInt32 {
  T val_;

public:
  inline explicit FromJSInt32(const Napi::Value &val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Expected a number");
    }
    val_ = static_cast<T>(val.ToNumber().Int32Value());
  }
  inline T Get() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSInt32 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSInt32(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<int32_t>(val_)); }
};

template <typename T> class FromJSUint32 {
  T val_;

public:
  inline explicit FromJSUint32(const Napi::Value &val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Expected a number");
    }
    val_ = static_cast<T>(val.ToNumber().Uint32Value());
  }
  inline T Get() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSUint32 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSUint32(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<uint32_t>(val_)); }
};

template <typename T> class FromJSInt64 {
  T val_;

public:
  inline explicit FromJSInt64(const Napi::Value &val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Expected a number");
    }
    val_ = static_cast<T>(val.ToNumber().Int64Value());
  }
  inline T Get() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSInt64 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSInt64(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<int64_t>(val_)); }
};

template <typename T> class FromJSDouble {
  T val_;

public:
  inline explicit FromJSDouble(const Napi::Value &val) {
    if (!val.IsNumber()) {
      throw Napi::TypeError::New(val.Env(), "Expected a number");
    }
    val_ = static_cast<T>(val.ToNumber().DoubleValue());
  }
  inline T Get() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSDouble {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSDouble(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<double>(val_)); }
};

#define TYPEMAPS_FOR_NUMBER(CTYPE, JSTYPE)                                                                             \
  template <> class FromJS<CTYPE> : public FromJS##JSTYPE<CTYPE> {                                                     \
  public:                                                                                                              \
    using FromJS##JSTYPE<CTYPE>::FromJS##JSTYPE;                                                                       \
  };                                                                                                                   \
                                                                                                                       \
  template <const ReturnAttribute &RETATTR> class ToJS<CTYPE, RETATTR> : public ToJS##JSTYPE<CTYPE, RETATTR> {         \
  public:                                                                                                              \
    using ToJS##JSTYPE<CTYPE, RETATTR>::ToJS##JSTYPE;                                                                  \
  }

TYPEMAPS_FOR_NUMBER(int, Int32);
TYPEMAPS_FOR_NUMBER(short, Int32);
TYPEMAPS_FOR_NUMBER(long, Int32);
TYPEMAPS_FOR_NUMBER(unsigned, Uint32);
TYPEMAPS_FOR_NUMBER(unsigned short, Uint32);
TYPEMAPS_FOR_NUMBER(unsigned long, Uint32);
TYPEMAPS_FOR_NUMBER(long long, Int64);
TYPEMAPS_FOR_NUMBER(unsigned long long, Int64);
TYPEMAPS_FOR_NUMBER(double, Double);
TYPEMAPS_FOR_NUMBER(float, Double);

} // namespace Typemap

} // namespace Nobind
