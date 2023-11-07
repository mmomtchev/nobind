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
  template <auto *FUNC, const ReturnAttribute &RETATTR = ReturnDefault> Module<MODULE> &def(const char *name) {
    Napi::Value (*wrapper)(const Napi::CallbackInfo &) = FunctionWrapper<RETATTR, FUNC>;
    Napi::Function js = Napi::Function::New(env_, wrapper);
    exports_.Set(name, js);
    return *this;
  }

  // Class
  template <class CLASS> ClassDefinition<CLASS> def(const char *name) {
    return ClassDefinition<CLASS>(name, env_, exports_, class_idx_++);
  }
};

} // namespace Nobind

#define NOBIND_MODULE(MODULE_NAME, MODULE_ARG)                                                                         \
  char const Nobind_##MODULE##_name[] = #MODULE_NAME;                                                                  \
  void Nobind_##MODULE##_Init_Wrapper(Nobind::Module<Nobind_##MODULE##_name> &);                                       \
  Napi::Object Nobind_##MODULE##_Init_Wrapper(Napi::Env, Napi::Object);                                                \
  NODE_API_MODULE(MODULE_NAME, Nobind_##MODULE##_Init_Wrapper)                                                         \
  Napi::Object Nobind_##MODULE##_Init_Wrapper(Napi::Env env, Napi::Object exports) {                                   \
    env.SetInstanceData(new Nobind::EnvInstanceData);                                                                  \
    Nobind::Module<Nobind_##MODULE##_name> m(env, exports);                                                            \
    Nobind_##MODULE##_Init_Wrapper(m);                                                                                 \
    return exports;                                                                                                    \
  }                                                                                                                    \
  void Nobind_##MODULE##_Init_Wrapper(Nobind::Module<Nobind_##MODULE##_name> &MODULE_ARG)
