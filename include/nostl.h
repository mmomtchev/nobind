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
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsArray()) {                                                                                            \
        throw Napi::TypeError::New(val.Env(), "Not an array");                                                         \
      }                                                                                                                \
      Napi::Array array = val.As<Napi::Array>();                                                                       \
      val_.reserve(array.Length());                                                                                    \
      for (size_t i = 0; i < array.Length(); i++) {                                                                    \
        val_.push_back(*FromJS<T>(array.Get(i)));                                                                      \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    inline VECTOR operator*() { return val_; }                                                                         \
  };                                                                                                                   \
                                                                                                                       \
  template <typename T> class ToJS<VECTOR> {                                                                           \
    Napi::Env env_;                                                                                                    \
    std::remove_cv_t<std::remove_reference_t<VECTOR>> val_;                                                            \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, VECTOR val) : env_(env), val_(val) {}                                          \
    inline Napi::Value operator*() {                                                                                   \
      Napi::Array array = Napi::Array::New(env_, val_.size());                                                         \
      for (size_t i = 0; i < val_.size(); i++) {                                                                       \
        array.Set(i, *ToJS<T>(env_, val_[i]));                                                                \
      }                                                                                                                \
      return array;                                                                                                    \
    }                                                                                                                  \
  };

#define TYPEMAPS_FOR_STD_MAP(MAP)                                                                                      \
  template <typename T> class FromJS<MAP> {                                                                            \
    std::remove_cv_t<std::remove_reference_t<MAP>> val_;                                                               \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit FromJS(Napi::Value val) {                                                                          \
      if (!val.IsObject()) {                                                                                           \
        throw Napi::TypeError::New(val.Env(), "Not an object");                                                        \
      }                                                                                                                \
      Napi::Object object = val.ToObject();                                                                            \
      for (auto prop : object) {                                                                                       \
        val_.insert({prop.first.ToString().Utf8Value(), *FromJS<T>(prop.second)});                                     \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    inline MAP operator*() { return val_; }                                                                            \
  };                                                                                                                   \
                                                                                                                       \
  template <typename T> class ToJS<MAP> {                                                                              \
    Napi::Env env_;                                                                                                    \
    std::remove_cv_t<std::remove_reference_t<MAP>> val_;                                                               \
                                                                                                                       \
  public:                                                                                                              \
    inline explicit ToJS(Napi::Env env, MAP val) : env_(env), val_(val) {}                                             \
    inline Napi::Value operator*() {                                                                                   \
      Napi::Object object = Napi::Object::New(env_);                                                                   \
      for (auto const &prop : val_) {                                                                                  \
        object.Set(Napi::String::New(env_, prop.first), *ToJS<T>(env_, prop.second));                                  \
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
