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
  FromJSInt32(const FromJSInt32 &) = delete;
  FromJSInt32(FromJSInt32 &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSInt32 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSInt32(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<int32_t>(val_)); }
  ToJSInt32(const ToJSInt32 &) = delete;
  ToJSInt32(ToJSInt32 &&) = default;
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
  FromJSUint32(const FromJSUint32 &) = delete;
  FromJSUint32(FromJSUint32 &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSUint32 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSUint32(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<uint32_t>(val_)); }
  ToJSUint32(const ToJSUint32 &) = delete;
  ToJSUint32(ToJSUint32 &&) = default;
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
  FromJSInt64(const FromJSInt64 &) = delete;
  FromJSInt64(FromJSInt64 &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSInt64 {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSInt64(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<int64_t>(val_)); }
  ToJSInt64(const ToJSInt64 &) = delete;
  ToJSInt64(ToJSInt64 &&) = default;
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
  FromJSDouble(const FromJSDouble &) = delete;
  FromJSDouble(FromJSDouble &&) = default;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJSDouble {
  Napi::Env env_;
  T val_;

public:
  inline explicit ToJSDouble(Napi::Env env, T val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::Number::New(env_, static_cast<double>(val_)); }
  ToJSDouble(const ToJSDouble &) = delete;
  ToJSDouble(ToJSDouble &&) = default;
};

#define TYPEMAPS_FOR_NUMBER(CTYPE, JSTYPE)                                                                             \
  template <> class FromJS<CTYPE> : public FromJS##JSTYPE<CTYPE> {                                                     \
  public:                                                                                                              \
    using FromJS##JSTYPE<CTYPE>::FromJS##JSTYPE;                                                                       \
    static const std::string TSType() { return "number"s; }                                                            \
  };                                                                                                                   \
                                                                                                                       \
  template <const ReturnAttribute &RETATTR> class ToJS<CTYPE, RETATTR> : public ToJS##JSTYPE<CTYPE, RETATTR> {         \
  public:                                                                                                              \
    using ToJS##JSTYPE<CTYPE, RETATTR>::ToJS##JSTYPE;                                                                  \
    static const std::string TSType() { return "number"s; }                                                            \
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
