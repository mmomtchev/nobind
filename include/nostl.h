#pragma once
#include <map>
#include <notypes.h>
#include <string>
#include <vector>

namespace Nobind {

namespace Typemap {

#define TYPEMAPS_FOR_STD_VECTOR(VECTOR)                                                                                \
  template <typename T> class FromJS<VECTOR> {                                                                         \
    std::remove_cv_t<std::remove_reference_t<VECTOR>> val_;                                                            \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(const Napi::Value &val) {                                                                   \
      if (!val.IsArray()) {                                                                                            \
        throw Napi::TypeError::New(val.Env(), "Expected an array");                                                    \
      }                                                                                                                \
      Napi::Array array = val.As<Napi::Array>();                                                                       \
      val_.reserve(array.Length());                                                                                    \
      for (size_t i = 0; i < array.Length(); i++) {                                                                    \
        val_.push_back(FromJSValue<T>(array.Get(i)).Get());                                                            \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    inline VECTOR Get() { return val_; }                                                                               \
  };                                                                                                                   \
                                                                                                                       \
  template <typename T, const ReturnAttribute &RETATTR> class ToJS<VECTOR, RETATTR> {                                  \
    Napi::Env env_;                                                                                                    \
    std::remove_cv_t<std::remove_reference_t<VECTOR>> val_;                                                            \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, VECTOR val) : env_(env), val_(val) {}                                          \
    inline Napi::Value Get() {                                                                                         \
      Napi::Array array = Napi::Array::New(env_, val_.size());                                                         \
      for (size_t i = 0; i < val_.size(); i++) {                                                                       \
        array.Set(i, ToJS<T, RETATTR>(env_, val_[i]).Get());                                                           \
      }                                                                                                                \
      return array;                                                                                                    \
    }                                                                                                                  \
  };

#define TYPEMAPS_FOR_STD_MAP(MAP)                                                                                      \
  template <typename T> class FromJS<MAP> {                                                                            \
    std::remove_cv_t<std::remove_reference_t<MAP>> val_;                                                               \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(const Napi::Value &val) {                                                                   \
      if (!val.IsObject()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Expected an object");                                                   \
      }                                                                                                                \
      Napi::Object object = val.ToObject();                                                                            \
      for (auto prop : object) {                                                                                       \
        val_.insert({prop.first.ToString().Utf8Value(), FromJSValue<T>(prop.second).Get()});                           \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    inline MAP Get() { return val_; }                                                                                  \
  };                                                                                                                   \
                                                                                                                       \
  template <typename T, const ReturnAttribute &RETATTR> class ToJS<MAP, RETATTR> {                                     \
    Napi::Env env_;                                                                                                    \
    std::remove_cv_t<std::remove_reference_t<MAP>> val_;                                                               \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, MAP val) : env_(env), val_(val) {}                                             \
    inline Napi::Value Get() {                                                                                         \
      Napi::Object object = Napi::Object::New(env_);                                                                   \
      for (auto const &prop : val_) {                                                                                  \
        object.Set(Napi::String::New(env_, prop.first), ToJS<T, RETATTR>(env_, prop.second).Get());                    \
      }                                                                                                                \
      return object;                                                                                                   \
    }                                                                                                                  \
  };

TYPEMAPS_FOR_STD_VECTOR(std::vector<T>);
TYPEMAPS_FOR_STD_VECTOR(std::vector<T> &);
TYPEMAPS_FOR_STD_VECTOR(const std::vector<T>);
TYPEMAPS_FOR_STD_VECTOR(const std::vector<T> &);

template <typename T> using string_map = std::map<std::string, T>;
TYPEMAPS_FOR_STD_MAP(string_map<T>);
TYPEMAPS_FOR_STD_MAP(string_map<T> &);
TYPEMAPS_FOR_STD_MAP(const string_map<T>);
TYPEMAPS_FOR_STD_MAP(const string_map<T> &);

} // namespace Typemap

} // namespace Nobind
