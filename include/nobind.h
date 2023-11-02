#pragma once
#include <napi.h>
#include <string>
#include <tuple>
#include <type_traits>

#include <noobject.h>
#include <notypes.h>

using namespace std::literals::string_literals;

namespace Nobind {

template <typename RETURN, typename... ARGS> class FunctionWrapper {
public:
  template <RETURN(FUNC)(ARGS...)> static Napi::Value ToJS(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    CheckArgLength<ARGS...>(env, info.Length());
    if constexpr (sizeof...(ARGS) > 0) {
      std::tuple<std::remove_const_t<std::decay_t<ARGS>>...> args;

      std::apply(
          [&info](auto &...args) {
            size_t i = 0;
            ((args = Typemap<std::remove_reference_t<decltype(args)>>::ToJS(info[i++])), ...);
          },
          args);

      RETURN result = std::apply(FUNC, args);
      return Typemap<RETURN>::FromJS(env, result);
    } else {
      RETURN result = FUNC();
      return Typemap<RETURN>::FromJS(env, result);
    }
  }
};

class Module {
  Napi::Env env_;
  Napi::Object exports_;

public:
  Module(Napi::Env env, Napi::Object exports) : env_(env), exports_(exports) {}

  // This rather interesting construct with two chained methods
  // allows to deduce the signature of a function out of an 'auto' template argument
  // C++17, tested to work with clang, gcc and MSVC
  // https://stackoverflow.com/questions/77404330/function-template-with-variable-argument-function-as-template-argument
  template <auto *fn> void def(const char *name) { def(name, std::integral_constant<decltype(fn), fn>{}); }

  // Global function
  template <typename RETURN, typename... ARGS, RETURN (*FN)(ARGS...)>
  void def(const char *name, std::integral_constant<RETURN (*)(ARGS...), FN>) {
    Napi::Value (*wrapper)(const Napi::CallbackInfo &) = FunctionWrapper<RETURN, ARGS...>::template ToJS<FN>;
    Napi::Function js = Napi::Function::New(env_, wrapper);
    exports_.Set(name, js);
  }

  // Class
  template <class CLASS> ClassDefinition<CLASS> def(const char *name) {
    return ClassDefinition<CLASS>(name, env_, exports_);
  }
};

} // namespace Nobind

#define NOBIND_MODULE(MODULE_NAME, MODULE_ARG)                                                                         \
  void Nobind_##MODULE##_Init_Wrapper(Nobind::Module &);                                                               \
  Napi::Object Nobind_##MODULE##_Init_Wrapper(Napi::Env, Napi::Object);                                                \
  NODE_API_MODULE(MODULE_NAME, Nobind_##MODULE##_Init_Wrapper)                                                         \
  Napi::Object Nobind_##MODULE##_Init_Wrapper(Napi::Env env, Napi::Object exports) {                                   \
    Nobind::Module m(env, exports);                                                                                    \
    Nobind_##MODULE##_Init_Wrapper(m);                                                                                 \
    return Napi::Object();                                                                                             \
  }                                                                                                                    \
  void Nobind_##MODULE##_Init_Wrapper(Nobind::Module &MODULE_ARG)
