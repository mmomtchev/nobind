#pragma once
#include <napi.h>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <notypes.h>

#include <noattributes.h>

#include <nofunction.h>

#include <nobuffer.h>
#include <nonumbermaps.h>
#include <noobject.h>
#include <nostl.h>
#include <nostringmaps.h>

namespace Nobind {

template <char const MODULE[]> class Module {
  Napi::Env env_;
  Napi::Object exports_;
  size_t class_idx_;

public:
  Module(Napi::Env env, Napi::Object exports) : env_(env), exports_(exports), class_idx_(0) {}

  // Global function
  template <auto *OBJECT, const ReturnAttribute &RET = ReturnDefault>
  std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(OBJECT)>>, Module<MODULE>> &def(const char *name) {
    Napi::Function::Callback wrapper;
    if constexpr (RET.isAsync()) {
      wrapper = FunctionWrapperAsync<RET, OBJECT>;
    } else {
      wrapper = FunctionWrapper<RET, OBJECT>;
    }
    Napi::Function js = Napi::Function::New(env_, wrapper);
    exports_.Set(name, js);
    return *this;
  }

  // Global getter/setter
  template <auto *OBJECT, const PropertyAttribute &PROP = ReadWrite>
  std::enable_if_t<!std::is_function_v<std::remove_pointer_t<decltype(OBJECT)>>, Module<MODULE> &>
  def(const char *name) {
    Napi::PropertyDescriptor::GetterCallback getter = &GetterWrapper<std::remove_pointer_t<decltype(OBJECT)>, OBJECT>;
    Napi::PropertyDescriptor::SetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &SetterWrapper<std::remove_pointer_t<decltype(OBJECT)>, OBJECT>;
    }
    exports_.DefineProperty(Napi::PropertyDescriptor::Accessor(name, getter, setter));
    return *this;
  }

  // Class
  template <class CLASS> ClassDefinition<CLASS> def(const char *name) {
    return ClassDefinition<CLASS>(name, env_, exports_, class_idx_++);
  }

  Napi::Env Env() { return env_; }

  Napi::Object Exports() { return exports_; }
};

} // namespace Nobind

#define NOBIND_MODULE_DATA(MODULE_NAME, MODULE_ARG, INSTANCE_DATA_TYPE)                                                \
  /* We make this assumption that is not guaranteed by the C++ specs but it seems to be true on all compilers */       \
  static_assert(&Nobind::BaseEnvInstanceData::_Nobind_cons ==                                                          \
                &Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>::_Nobind_cons);                                           \
  char const Nobind_##MODULE_NAME##_name[] = #MODULE_NAME;                                                             \
  void Nobind_##MODULE_NAME##_Init_Wrapper(Nobind::Module<Nobind_##MODULE_NAME##_name> &);                             \
  Napi::Object Nobind_##MODULE_NAME##_Init_Wrapper(Napi::Env, Napi::Object);                                           \
  NODE_API_MODULE(MODULE_NAME, Nobind_##MODULE_NAME##_Init_Wrapper)                                                    \
  Napi::Object Nobind_##MODULE_NAME##_Init_Wrapper(Napi::Env env, Napi::Object exports) {                              \
    env.SetInstanceData(new Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>);                                              \
    Nobind::Module<Nobind_##MODULE_NAME##_name> m(env, exports);                                                       \
    Nobind_##MODULE_NAME##_Init_Wrapper(m);                                                                            \
    return exports;                                                                                                    \
  }                                                                                                                    \
  void Nobind_##MODULE_NAME##_Init_Wrapper(Nobind::Module<Nobind_##MODULE_NAME##_name> &MODULE_ARG)

#define NOBIND_MODULE(MODULE_NAME, MODULE_ARG) NOBIND_MODULE_DATA(MODULE_NAME, MODULE_ARG, Nobind::EmptyEnvInstanceData)
