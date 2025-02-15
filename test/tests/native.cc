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

struct PerIsolateData {
  Napi::ObjectReference exports;
};

Napi::Value get_exports(const Napi::CallbackInfo &info) {
  Napi::Env env{info.Env()};
  return env.GetInstanceData<Nobind::EnvInstanceData<PerIsolateData>>()->exports.Value();
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

NOBIND_MODULE_DATA(native, m, PerIsolateData) {
  m.def<WithNative>("WithNative").cons<>().def<&WithNative::method_native>("method_native");
  m.def<&global_native>("global_native");

  m.Env().GetInstanceData<Nobind::EnvInstanceData<PerIsolateData>>()->exports =
      Napi::Persistent<Napi::Object>(m.Exports());
  m.Exports().Set("get_exports", Napi::Function::New(m.Env(), get_exports));

  m.typescript_fragment("export const debug_build;\n");
  m.Exports().Set("debug_build", Napi::Boolean::New(m.Env(),
#ifdef DEBUG
                                                    true
#else
                                                    false
#endif
                                                    ));
}
