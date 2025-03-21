#include <fixtures/basic_class.h>

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

Napi::Value get_string(Napi::Env env) { return Napi::String::New(env, "hello from get_string"); }

// Native extension function with This() as Napi::Value
Napi::Value native_extension(Napi::Value val) { return val; }

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

constexpr bool BasicFinalizers =
#ifdef NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER
    true
#else
    false
#endif
    ;

NOBIND_MODULE_DATA(native, m, PerIsolateData) {
  m.def<WithNative>("WithNative")
      .cons<>()
      .def<&WithNative::method_native>("method_native")
      .ext<&native_extension>("native_extension");
  m.def<&global_native>("global_native");
  m.def<&get_string>("get_string");

  m.Env().GetInstanceData<Nobind::EnvInstanceData<PerIsolateData>>()->exports =
      Napi::Persistent<Napi::Object>(m.Exports());
  m.Exports().Set("get_exports", Napi::Function::New(m.Env(), get_exports));

  m.def<&BasicFinalizers, Nobind::ReadOnly>("basic_finalizers");

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
  m.typescript_fragment("export const debug_build;\n");
#endif
  m.Exports().Set("debug_build", Napi::Boolean::New(m.Env(),
#ifdef DEBUG
                                                    true
#else
                                                    false
#endif
                                                    ));
}
