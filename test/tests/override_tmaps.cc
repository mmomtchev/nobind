#include <fixtures/global_functions.h>

#include <nooverrides.h>

namespace Nobind {
namespace TypemapOverrides {
template <> class FromJS<int> {
  int val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    val_ = std::atoi(val.ToString().Utf8Value().c_str());
  }
  inline int Get() { return val_; }
};

template <> class ToJS<int> {
  Napi::Env env_;
  int val_;

public:
  inline explicit ToJS(Napi::Env env, int val) : env_(env), val_(val) {}
  inline Napi::Value Get() { return Napi::String::New(env_, std::to_string(val_)); }
};

template <> class FromJS<const std::string &> {
  static const std::string fixed;

public:
  inline explicit FromJS(Napi::Value) {}
  inline const std::string &Get() { return fixed; }
  static const size_t Inputs = 0;
};

const std::string FromJS<const std::string &>::fixed = "Static string";

} // namespace TypemapOverrides

} // namespace Nobind

#include <nobind.h>

NOBIND_MODULE(override_tmaps, m) {
  m.def<&add>("add");
  m.def<&hello>("hello");
}
