#pragma once
#include <napi.h>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <notypes.h>

#include <noattributes.h>

#include <nobuffer.h>
#include <nonumbermaps.h>
#include <noobject.h>
#include <nostl.h>
#include <nostringmaps.h>

namespace Nobind {

// This is a 3-stage version of a trick using std::integral_constant which is proposed here:
// https://stackoverflow.com/questions/77404330/function-template-with-variable-argument-function-as-template-argument
template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...), std::size_t... I>
inline Napi::Value FunctionWrapper(const Napi::CallbackInfo &info, std::integral_constant<RETURN (*)(ARGS...), FUNC>,
                                   std::index_sequence<I...>) {
  Napi::Env env = info.Env();

  CheckArgLength<ARGS...>(env, info.Length());
  try {
    if constexpr (sizeof...(ARGS) > 0) {
      // Call the FromJS constructors
      std::tuple<Nobind::Typemap::FromJS<ARGS>...> args(Nobind::FromJS<ARGS>(info[I])...);
      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        FUNC(*std::get<I>(args)...);
        return env.Undefined();
        // FromJS objects are destroyed
      } else {
        // Convert and call
        RETURN result = FUNC(*std::get<I>(args)...);
        // Call the ToJS constructor
        auto output = Nobind::Typemap::ToJS<RETURN, RETATTR>(env, result);
        // Convert
        return *output;
        // FromJS/ToJS objects are destroyed
      }
    } else {
      if constexpr (std::is_void_v<RETURN>) {
        // Call
        FUNC();
        return env.Undefined();
      } else {
        // Call
        RETURN result = FUNC();
        // Call the ToJS constructor
        auto output = Nobind::Typemap::ToJS<RETURN, RETATTR>(env, result);
        // Convert
        return *output;
        // ToJS object is destroyed
      }
    }
  } catch (const std::exception &e) {
    throw Napi::Error::New(env, e.what());
  }
}

template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...)>
inline Napi::Value FunctionWrapper(const Napi::CallbackInfo &info, std::integral_constant<RETURN (*)(ARGS...), FUNC>) {
  return FunctionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                  std::index_sequence_for<ARGS...>{});
}

// This is the function that gets instantiated to create a wrapper (by getting a pointer)
// and gets will be called by JavaScript
template <const ReturnAttribute &RETATTR = ReturnDefault, auto *FUNC>
Napi::Value FunctionWrapper(const Napi::CallbackInfo &info) {
  return FunctionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{});
}

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
