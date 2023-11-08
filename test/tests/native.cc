#include <fixtures/basic_class.h>

#include <napi.h>
#include <nobind.h>

Napi::Value global_native(Napi::Value val) {
  Napi::Env env(val.Env());
  if (!val.IsString())
    throw Napi::Error::New(env, "Expected a string");
  std::string r{val.ToString().Utf8Value() + " from global_native"};
  return Napi::String::New(env, r);
}

class WithNative {
public:
  WithNative() {}
  Napi::Value method_native(Napi::Value val) {
    Napi::Env env(val.Env());
    if (!val.IsString())
      throw Napi::Error::New(env, "Expected a string");
    std::string r{val.ToString().Utf8Value() + " from method_native"};
    return Napi::String::New(env, r);
  }
};

NOBIND_MODULE(native, m) {
  m.def<WithNative>("WithNative").cons<>().def<&WithNative::method_native>("method_native");
  m.def<&global_native>("global_native");

  m.Exports().Set("debug_build", Napi::Boolean::New(m.Env(),
#ifdef DEBUG
                                                    true
#else
                                                    false
#endif
                                                    ));
}
