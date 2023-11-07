#pragma once
#include <napi.h>
#include <noattributes.h>

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

} // namespace Nobind
