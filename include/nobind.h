#pragma once

#include <nonapi.h>

#include <nodebug.h>

#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <nohelpers.h>
#include <notypes.h>

#include <noattributes.h>

#include <nofunction.h>

#include <nobuffer.h>
#include <noiterator.h>
#include <nonumbermaps.h>
#include <noobject.h>
#include <nosmartptr.h>
#include <nostl.h>
#include <nostringmaps.h>

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
#include <notypescript.h>
#endif

namespace Nobind {

template <char const MODULE[]> class Module {
  Napi::Env env_;
  Napi::Object exports_;
  size_t class_idx_;
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
  std::string typescript_types_;
#endif

public:
  Module(Napi::Env env, Napi::Object exports)
      : env_{env}, exports_{exports}, class_idx_{0}
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
        ,
        typescript_types_{""}
#endif
  {
  }

  ~Module() noexcept(false) {
    auto instance = env_.GetInstanceData<BaseEnvInstanceData>();
    NOBIND_VERBOSE(INIT, "Environment setup for %p\n", instance);
#ifndef NOBIND_NO_OBJECT_STORE
    instance->_Nobind_object_store = new ObjectStore<void *>(class_idx_);
#endif
    auto r = napi_add_async_cleanup_hook(
        env_,
        [](napi_async_cleanup_hook_handle hook, void *arg) {
          auto instance = static_cast<BaseEnvInstanceData *>(arg);
          NOBIND_VERBOSE(INIT, "Environment cleanup hook for %p\n", instance);
          NOBIND_ASSERT(instance->_Nobind_environment_cleanup_hook == hook);
#ifndef NOBIND_NO_OBJECT_STORE
          delete instance->_Nobind_object_store;
          instance->_Nobind_object_store = nullptr;
#endif
          uv_close(reinterpret_cast<uv_handle_t *>(instance->_Nobind_js_thread_async_handle), [](uv_handle_t *async) {
            auto instance = static_cast<BaseEnvInstanceData *>(async->data);
            NOBIND_VERBOSE(INIT, "Environment cleanup hook bottom half for %p\n", instance);
            delete reinterpret_cast<uv_async_t *>(async);
            auto r = napi_remove_async_cleanup_hook(instance->_Nobind_environment_cleanup_hook);
            if (r != napi_ok) {
              fprintf(stderr, "Failed to destroy async handle\n");
              std::abort();
            }
          });
        },
        instance, &instance->_Nobind_environment_cleanup_hook);
    if (r != napi_ok) {
      throw Napi::Error::New(env_, "Failed to register Object Store cleanup hook");
    }
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    exports_.DefineProperty(Napi::PropertyDescriptor::Value(NOBIND_TYPESCRIPT_PROP,
                                                            Napi::String::New(env_, typescript_types_), napi_default));
#endif
  }

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
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    typescript_types_ += FunctionSignature<RET, OBJECT>(name, "export function ");
#endif
    return *this;
  }

  // Global function / sync+async shortcut
  template <auto OBJECT, const ReturnAttribute &RET = ReturnDefault>
  std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(OBJECT)>>, Module<MODULE>> &
  def(const char *name_sync, const char *name_async) {
    static_assert(!RET.isAsync(), "Do not specify async with the duplex definition");
    def<OBJECT, RET>(name_sync);
    def<OBJECT, RetWithAsync<RET>>(name_async);
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

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    typescript_types_ += GlobalSignature<PROP, std::remove_pointer_t<decltype(OBJECT)>>(name, "export ");
#endif
    return *this;
  }

  // Class definition
  // (this is a use case for std::bases but it seems that the proposal has lost traction)
  template <typename CLASS, typename BASE = void, typename... INTERFACES>
  ClassDefinition<CLASS, BASE, INTERFACES...> def(const char *name) {
    return ClassDefinition<CLASS, BASE, INTERFACES...>(name, env_, exports_, class_idx_++
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
                                                       ,
                                                       typescript_types_
#endif
    );
  }

  // Forward class declaration
  template <class CLASS> void decl(const char *name) {
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    NoObjectWrap<CLASS>::Declare(name);
#endif
  };

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
  // Custom TypeScript fragment
  void typescript_fragment(const char *fragment) { typescript_types_ += fragment; }
  void typescript_fragment(const std::string &fragment) { typescript_types_ += fragment; }
#endif

  Napi::Env Env() { return env_; }

  Napi::Object Exports() { return exports_; }
};

} // namespace Nobind

#define NOBIND_MODULE_DATA(MODULE_NAME, MODULE_ARG, INSTANCE_DATA_TYPE)                                                \
  /* We make this assumption that is not guaranteed by the C++ specs but it seems to be true on all compilers */       \
  static_assert(&Nobind::BaseEnvInstanceData::_Nobind_cons ==                                                          \
                &Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>::_Nobind_cons);                                           \
  static_assert(&Nobind::BaseEnvInstanceData::_Nobind_js_thread ==                                                     \
                &Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>::_Nobind_js_thread);                                      \
  char const Nobind_##MODULE_NAME##_name[] = #MODULE_NAME;                                                             \
  void Nobind_##MODULE_NAME##_Init_Wrapper(Nobind::Module<Nobind_##MODULE_NAME##_name> &);                             \
  Napi::Object Nobind_##MODULE_NAME##_Init_Wrapper(Napi::Env, Napi::Object);                                           \
  NODE_API_MODULE(MODULE_NAME, Nobind_##MODULE_NAME##_Init_Wrapper)                                                    \
  Napi::Object Nobind_##MODULE_NAME##_Init_Wrapper(Napi::Env env, Napi::Object exports) {                              \
    NOBIND_DEBUG_INIT(#MODULE_NAME);                                                                                   \
    auto instance = new Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>;                                                   \
    instance->_Nobind_js_thread = std::this_thread::get_id();                                                          \
    env.SetInstanceData(instance);                                                                                     \
    Nobind::InitMainThreadQueue<Nobind::EnvInstanceData<INSTANCE_DATA_TYPE>>(env);                                     \
    Nobind::Module<Nobind_##MODULE_NAME##_name> m{env, exports};                                                       \
    Nobind_##MODULE_NAME##_Init_Wrapper(m);                                                                            \
    return exports;                                                                                                    \
  }                                                                                                                    \
  void Nobind_##MODULE_NAME##_Init_Wrapper(Nobind::Module<Nobind_##MODULE_NAME##_name> &MODULE_ARG)

#define NOBIND_MODULE(MODULE_NAME, MODULE_ARG) NOBIND_MODULE_DATA(MODULE_NAME, MODULE_ARG, Nobind::EmptyEnvInstanceData)
