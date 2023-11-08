#include <fixtures/global_functions.h>

#include <nooverrides.h>

namespace Nobind {
namespace TypemapOverrides {
template <> class FromJS<int> {
  int val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Not a string");
    }
    val_ = std::atoi(val.ToString().Utf8Value().c_str());
  }
  inline int operator*() { return val_; }

  static constexpr bool enable = true;
};

template <const ReturnAttribute &RETATTR> class ToJS<int, RETATTR> {
  Napi::Env env_;
  int val_;

public:
  inline explicit ToJS(Napi::Env env, int val) : env_(env), val_(val) {}
  inline Napi::Value operator*() { return Napi::String::New(env_, std::to_string(val_)); }

  static constexpr bool enable = true;
};
} // namespace OverridesTypemap

} // namespace Nobind

#include <nobind.h>

NOBIND_MODULE(override_tmaps, m) {
  m.def<&add>("add");
}
