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

  try {
    // Call the FromJS constructors
    //
    size_t idx = 0;
    std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
    CheckArgLength(env, idx, info.Length());
    if constexpr (std::is_void_v<RETURN>) {
      // Convert and call
      FUNC(std::get<I>(args).Get()...);
      return env.Undefined();
      // FromJS objects are destroyed
    } else {
      // Convert and call
      RETURN result = FUNC(std::get<I>(args).Get()...);
      // Call the ToJS constructor
      auto output = ToJS_t<RETURN, RETATTR>(env, result);
      // Convert
      return output.Get();
      // FromJS/ToJS objects are destroyed
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
  FunctionWrapperTasklet(Napi::Env env, Napi::Promise::Deferred deferred, std::tuple<FromJS_t<ARGS>...> &&args)
      : AsyncWorker(env, "nobind_AsyncWorker"), env_(env), deferred_(deferred), output(), args_(std::move(args)) {}

  template <std::size_t... I> void ExecuteImpl(std::index_sequence<I...>) {
    try {
      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        FUNC(std::get<I>(args_).Get()...);
      } else {
        // Convert and call
        RETURN result = FUNC(std::get<I>(args_).Get()...);
        // Call the ToJS constructor
        output = std::make_unique<ToJS_t<RETURN, RETATTR>>(env_, result);
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
      try {
        auto result = output->Get();
        deferred_.Resolve(result);
      } catch (const std::exception &e) {
        deferred_.Reject(Napi::String::New(env_, e.what()));
      }
    }
  }

  virtual void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }
};

template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...)>
inline Napi::Value FunctionWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (*)(ARGS...), FUNC>) {
  Napi::Env env = info.Env();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  try {
    size_t idx = 0;
    // Alas, std::forward_as_tuple does not guarantee
    // the evaluation order of its arguments, only *braced-init-list* lists do
    // https://en.cppreference.com/w/cpp/language/list_initialization
    auto tasklet =
        new FunctionWrapperTasklet<RETATTR, FUNC, RETURN, ARGS...>(env, deferred, {FromJSArgs<ARGS>(info, idx)...});

    try {
      CheckArgLength(env, idx, info.Length());
    } catch (...) {
      delete tasklet;
      std::rethrow_exception(std::current_exception());
    }

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

// This is the async function that gets instantiated to create a wrapper (by getting a pointer)
// and gets will be called by JavaScript
template <const ReturnAttribute &RETATTR = ReturnDefault, auto *FUNC>
Napi::Value FunctionWrapperAsync(const Napi::CallbackInfo &info) {
  return FunctionWrapperAsync<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{});
}

// Global or class static getter wrapper
template <typename T, T *OBJECT> static Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return ToJS<T, ReturnDefault>(env, *OBJECT).Get();
}

// Global or class static setter wrapper
template <typename T, T *OBJECT> static void SetterWrapper(const Napi::CallbackInfo &info) {
  *OBJECT = FromJSValue<T>(info[0]).Get();
}

} // namespace Nobind
