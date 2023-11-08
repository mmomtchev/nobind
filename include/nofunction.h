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
      std::tuple<FromJS_t<ARGS>...> args{Nobind::FromJS<ARGS>(info[I])...};
      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        FUNC(*std::get<I>(args)...);
        return env.Undefined();
        // FromJS objects are destroyed
      } else {
        // Convert and call
        RETURN result = FUNC(*std::get<I>(args)...);
        // Call the ToJS constructor
        auto output = ToJS_t<RETURN, RETATTR>(env, result);
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
        auto output = ToJS_t<RETURN, RETATTR>(env, result);
        // Convert
        return *output;
        // ToJS object is destroyed
      }
    }
  } catch (const std::exception &e) {
    throw Napi::Error::New(env, e.what());
  }
}

template <const ReturnAttribute &RETATTR, auto *FUNC, typename RETURN, typename... ARGS>
class FunctionWrapperTasklet : public Napi::AsyncWorker {
  Napi::Env env_;
  Napi::Promise::Deferred deferred_;
  std::unique_ptr<ToJS_t<RETURN, RETATTR>> output;
  std::tuple<FromJS_t<ARGS>...> args_;

public:
  FunctionWrapperTasklet(Napi::Env env, Napi::Promise::Deferred deferred, const std::tuple<FromJS_t<ARGS>...> &&args)
      : AsyncWorker(env, "nobind_AsyncWorker"), env_(env), deferred_(deferred), output(), args_(std::move(args)) {}

  template <std::size_t... I> void ExecuteImpl(std::index_sequence<I...>) {
    try {
      if constexpr (sizeof...(ARGS) > 0) {
        if constexpr (std::is_void_v<RETURN>) {
          // Convert and call
          FUNC(*std::get<I>(args_)...);
        } else {
          // Convert and call
          RETURN result = FUNC(*std::get<I>(args_)...);
          // Call the ToJS constructor
          output = std::make_unique<ToJS_t<RETURN, RETATTR>>(env_, result);
        }
      } else {
        if constexpr (std::is_void_v<RETURN>) {
          // Call
          FUNC();
        } else {
          // Call
          RETURN result = FUNC();
          // Call the ToJS constructor
          output = std::make_unique<ToJS_t<RETURN, RETATTR>>(env_, result);
        }
      }
    } catch (const std::exception &e) {
      SetError(e.what());
    }
  }

  virtual void Execute() override { ExecuteImpl(std::index_sequence_for<ARGS...>{}); }

  virtual void OnOK() override {
    if constexpr (std::is_void_v<RETURN>) {
      deferred_.Resolve(env_.Undefined());
    } else {
      auto result = **output;
      deferred_.Resolve(result);
    }
  }

  virtual void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }
};

template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...), std::size_t... I>
inline Napi::Value FunctionWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (*)(ARGS...), FUNC>, std::index_sequence<I...>) {
  Napi::Env env = info.Env();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  try {
    CheckArgLength<ARGS...>(env, info.Length());

    auto tasklet = new FunctionWrapperTasklet<RETATTR, FUNC, RETURN, ARGS...>(
        env, deferred, std::forward_as_tuple(Nobind::FromJS<ARGS>(info[I])...));

    tasklet->Queue();
  } catch (const std::exception &e) {
    deferred.Reject(Napi::Error::New(env, e.what()).Value());
  }
  return deferred.Promise();
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

template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...)>
inline Napi::Value FunctionWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (*)(ARGS...), FUNC>) {
  return FunctionWrapperAsync<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                       std::index_sequence_for<ARGS...>{});
}

// This is the async function that gets instantiated to create a wrapper (by getting a pointer)
// and gets will be called by JavaScript
template <const ReturnAttribute &RETATTR = ReturnDefault, auto *FUNC>
Napi::Value FunctionWrapperAsync(const Napi::CallbackInfo &info) {
  return FunctionWrapperAsync<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{});
}

// Global or class static getter wrapper
template <typename T, T *OBJECT> static Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return *ToJS<T, ReturnDefault>(env, *OBJECT);
}

// Global or class static setter wrapper
template <typename T, T *OBJECT> static void SetterWrapper(const Napi::CallbackInfo &info) {
  *OBJECT = *FromJS<T>(info[0]);
}

} // namespace Nobind
