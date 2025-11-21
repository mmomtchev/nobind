#pragma once
#ifndef NOBIND_NAME_NOT_INITIALIZED
#define NOBIND_NAME_NOT_INITIALIZED "unknown /* may be missing a forward declaration */"
#endif

#include <assert.h>
#include <functional>
#include <iostream>
#include <nonapi.h>
#include <numeric>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>

#include <nodebug.h>
#include <nofunction.h>
#include <noobjectstore.h>
#include <notypes.h>
#include <notypescript.h>

#include <uv.h>

using namespace std::literals::string_literals;

namespace Nobind {

struct EmptyEnvInstanceData {};

struct BaseEnvInstanceData {
#ifndef NOBIND_NO_OBJECT_STORE
  ObjectStore<void *> *_Nobind_object_store;
#endif
  std::thread::id _Nobind_js_thread;
  uv_async_t _Nobind_js_thread_async_handle;
  std::queue<std::function<void()>> _Nobind_js_thread_jobs;
  std::mutex _Nobind_js_thread_jobs_lock;
  // Per-environment constructors for all proxied types
  std::vector<Napi::FunctionReference> _Nobind_cons;

  ~BaseEnvInstanceData() {
    NOBIND_VERBOSE(INIT, "Destroy instance data, jobs on the queue %d\n", _Nobind_js_thread_jobs.size());
  }
};

template <typename T> struct EnvInstanceData : BaseEnvInstanceData, public T {};

// The JS proxy object type
template <typename CLASS> class NoObjectWrap : public Napi::ObjectWrap<NoObjectWrap<CLASS>> {
  template <typename T> friend class Typemap::FromJS;
  template <typename T, const ReturnAttribute &RETATTR> friend class Typemap::ToJS;

  // Async worker for async class methods, the wrapper is a private method below
  template <const ReturnAttribute &RETATTR, typename BASE, auto FUNC, typename RETURN, typename... ARGS>
  class MethodWrapperTasklet : public Napi::AsyncWorker {
    Napi::Env env_;
    Napi::Promise::Deferred deferred_;
    std::unique_ptr<ToJS_t<RETURN, RETATTR>> output;
    // FromJS wrappers also contain persistent references to their underlying JS values
    std::tuple<FromJS_t<ARGS>...> args_;
#ifndef NOBIND_NO_ASYNC_LOCKING
    // This is the This typemap, used only for locking
    // (Node-API sets the C++ this pointer for us)
    FromJS_t<CLASS> this_tm_;
#endif
    // This is the This persistent
    Napi::ObjectReference this_ref;
    // This is the This wrapper
    NoObjectWrap<CLASS> *wrapper_;
    BASE *self_;

  public:
    MethodWrapperTasklet(Napi::Env env, Napi::Promise::Deferred deferred, CLASS *self, NoObjectWrap<CLASS> *wrapper,
#ifndef NOBIND_NO_ASYNC_LOCKING
                         FromJS_t<CLASS> &&this_tm,
#endif
                         std::tuple<FromJS_t<ARGS>...> &&args)
        : AsyncWorker(env, "nobind_AsyncWorker"), env_(env), deferred_(deferred), output(), args_(std::move(args)),
#ifndef NOBIND_NO_ASYNC_LOCKING
          this_tm_(std::move(this_tm)),
#endif
          this_ref(Napi::Persistent(wrapper->Value())), wrapper_(wrapper), self_(static_cast<BASE *>(self)) {
    }

    template <std::size_t... I> void ExecuteImpl(std::index_sequence<I...>) {
#ifndef NOBIND_NO_ASYNC_LOCKING
      FromJSLockGuard<CLASS> this_lock_guard{this_tm_};
      [[maybe_unused]] std::tuple<FromJSLockGuard<ARGS>...> lock_guards{std::get<I>(args_)...};
#endif

      try {
        if constexpr (std::is_void_v<RETURN>) {
          // Convert and call
          (self_->*FUNC)(std::get<I>(args_).Get()...);
        } else {
          // Convert and call
          RETURN result = (self_->*FUNC)(std::get<I>(args_).Get()...);
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
          deferred_.Resolve(wrapper_->SetupNested<RETATTR>(output->Get()));
        } catch (const std::exception &e) {
          deferred_.Reject(Napi::String::New(env_, e.what()));
        }
      }
    }

    virtual void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }
  };

public:
  using Finalizer = std::function<void(Napi::BasicEnv, CLASS *)>;

  // JS convention constructor
  NoObjectWrap(const Napi::CallbackInfo &);
  // C++ convention constructors
  template <bool OWNED> static Napi::Value New(Napi::Env, CLASS *, Finalizer = Finalizer{});
  template <bool OWNED> static Napi::Value New(Napi::Env, const CLASS *);
  virtual ~NoObjectWrap() override;
#ifdef NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER
  virtual void Finalize(Napi::BasicEnv) override;
#endif
  static Napi::Function GetClass(Napi::Env, const char *,
                                 const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &);

  // Confirm the instance against the constructor, throws on error
  static void CheckInstance(Napi::Value);
  // Retrieve the C++ object pointer
  CLASS *Get();
#ifndef NOBIND_NO_ASYNC_LOCKING
  // Acquire the async lock (may block)
  void Lock() NOBIND_NOEXCEPT;
  // Release the async lock
  void Unlock() NOBIND_NOEXCEPT;
#endif

  // Constructor wrapper, these are only a pair - there are no pointers to constructors in C++
  template <typename... ARGS> void ConsWrapper(const Napi::CallbackInfo &info) {
    ConsWrapper<ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  // The first function of the member method wrapper trio (same std::integral_constant trick)
  // This is the function that gets instantiated to create a wrapper (by getting a pointer)
  // and gets will be called by JavaScript
  template <const ReturnAttribute &RET = ReturnDefault, auto FUNC>
  Napi::Value MethodWrapper(const Napi::CallbackInfo &info) {
    return MethodWrapper<RET>(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  // The first function of the async member trio
  // This is the function that gets instantiated to create a wrapper (by getting a pointer)
  // and gets will be called by JavaScript
  template <const ReturnAttribute &RET = ReturnDefault, auto FUNC>
  Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info) {
    return MethodWrapperAsync<RET>(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  // Extension wrapper, 3 stages, this is the first one
  template <const ReturnAttribute &RET = ReturnDefault, auto FUNC>
  Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info) {
    return ExtensionWrapper<RET>(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  template <typename T, T CLASS::*MEMBER> Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
#ifndef NOBIND_NO_ASYNC_LOCKING
    // Lock this
    FromJS_t<CLASS> this_tm = FromJSValue<CLASS>(info.This());
    FromJSLockGuard<CLASS> this_lock_guard(this_tm);
#endif
    if constexpr (std::is_scalar_v<T>)
      // Copy scalar objects
      return ToJS<T, ReturnNested>(env, self->*MEMBER).Get();
    else
      // Return a nested reference
      return SetupNested<ReturnNested>(ToJS<T &, ReturnNested>(env, self->*MEMBER).Get());
  }

  template <typename T, T CLASS::*MEMBER> void SetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    auto tm = FromJSValue<T>(val);
#ifndef NOBIND_NO_ASYNC_LOCKING
    // Lock this
    FromJS_t<CLASS> this_tm = FromJSValue<CLASS>(info.This());
    FromJSLockGuard<CLASS> this_guard{this_tm};
    FromJSLockGuard<T> val_guard{tm};
#endif
    self->*MEMBER = tm.Get();
  }

  template <typename T, T *MEMBER>
  static void StaticSetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    auto tm = FromJSValue<T>(val);
#ifndef NOBIND_NO_ASYNC_LOCKING
    FromJSLockGuard<T> guard{tm};
#endif
    *MEMBER = tm.Get();
  }

  static void Declare(const char *jsname) { name = std::string{jsname}; }

  static void
  Configure(const std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> &constructors,
            size_t idx) {
    // (class_idx == 0) - first module initialization
    // (class_idx == idx) - subsequent initialization (worker_thread)
    assert(class_idx == 0 || class_idx == idx);
    class_idx = idx;
    cons = constructors;
  }

  static const std::string &GetName() { return name; }

private:
  // The two remaining functions of the member method wrapper trio
  // The first (second of the three) has 4 possibles signatures:
  // - regular, const, noexcept and const noexcept
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...)>
  NOBIND_INLINE Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...), FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const>
  NOBIND_INLINE Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) const, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) noexcept>
  NOBIND_INLINE Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) noexcept, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const noexcept>
  NOBIND_INLINE Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) const noexcept, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  // The last one of the trio
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, auto FUNC, typename... ARGS,
            std::size_t... I>
  NOBIND_INLINE Napi::Value MethodWrapper(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    size_t idx = 0;
    try {
      // Call the FromJS constructors
      std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
      CheckArgLength(env, idx, info.Length());
#ifndef NOBIND_NO_ASYNC_LOCKING
      // Lock this
      FromJS_t<CLASS> this_tm = FromJSValue<CLASS>(info.This());
      FromJSLockGuard<CLASS> this_guard{this_tm};
      [[maybe_unused]] std::tuple<FromJSLockGuard<ARGS>...> release_guards{std::get<I>(args)...};
#endif

      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        (static_cast<BASE *>(self)->*FUNC)(std::get<I>(args).Get()...);
        return env.Undefined();
        // FromJS objects are destroyed
      } else {
        // Convert and call
        RETURN result = (static_cast<BASE *>(self)->*FUNC)(std::get<I>(args).Get()...);
        // Call the ToJS constructor
        auto output = ToJS_t<RETURN, RETATTR>(env, result);
        // Convert
        return SetupNested<RETATTR>(output.Get());
        // FromJS/ToJS objects are destroyed
      }
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  // The two remaining functions of the member async method wrapper trio (the first one with its 4 signatures)
  // (BASE == CLASS unless calling an inherited method, in this case it is the class defining it)
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...)>
  NOBIND_INLINE Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                               std::integral_constant<RETURN (BASE::*)(ARGS...), FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const>
  NOBIND_INLINE Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                               std::integral_constant<RETURN (BASE::*)(ARGS...) const, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) noexcept>
  NOBIND_INLINE Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                               std::integral_constant<RETURN (BASE::*)(ARGS...) noexcept, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const noexcept>
  NOBIND_INLINE Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                               std::integral_constant<RETURN (BASE::*)(ARGS...) const noexcept, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  // The actual wrapper for async class methods
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, auto FUNC, typename... ARGS,
            std::size_t... I>
  NOBIND_INLINE Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

#if _MSC_VER && !__INTEL_COMPILER
// MSVC doesn't appreciate the initialization sequence of Napi::Promise::Deferred
#pragma warning(push)
#pragma warning(disable : 6001)
#endif
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    try {
      size_t idx = 0;
      // Alas, std::forward_as_tuple does not guarantee
      // the evaluation order of its arguments, only *braced-init-list* lists do
      // https://en.cppreference.com/w/cpp/language/list_initialization
      auto tasklet = new MethodWrapperTasklet<RETATTR, BASE, FUNC, RETURN, ARGS...>(env, deferred, self, this,
#ifndef NOBIND_NO_ASYNC_LOCKING
                                                                                    FromJSValue<CLASS>(info.This()),
#endif
                                                                                    {FromJSArgs<ARGS>(info, idx)...});
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
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning(pop)
#endif
  }

  // The constructor wrapper implementation
  template <typename... ARGS, std::size_t... I>
  NOBIND_INLINE void ConsWrapper(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    // Call the FromJS constructors
    size_t idx = 0;
    std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
    CheckArgLength(env, idx, info.Length());
#ifndef NOBIND_NO_ASYNC_LOCKING
    [[maybe_unused]] std::tuple<FromJSLockGuard<ARGS>...> release_guards{std::get<I>(args)...};
#endif

    // Convert and call
    self = new CLASS(std::get<I>(args).Get()...);
  }

  // The extension wrapper, it adds an additional first argument by converting info.This()
  // Three stages, second stage, This() is CLASS &
  template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(CLASS &, ARGS...)>
  NOBIND_INLINE Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                             std::integral_constant<RETURN (*)(CLASS &, ARGS...), FUNC>) {
    return ExtensionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                     std::index_sequence_for<ARGS...>{});
  }
  // Second stage, This() is const CLASS &
  template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(const CLASS &, ARGS...)>
  NOBIND_INLINE Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                             std::integral_constant<RETURN (*)(const CLASS &, ARGS...), FUNC>) {
    return ExtensionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                     std::index_sequence_for<ARGS...>{});
  }
  // Second stage, This() is Napi::Value
  template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(Napi::Value, ARGS...)>
  NOBIND_INLINE Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                             std::integral_constant<RETURN (*)(Napi::Value, ARGS...), FUNC>) {
    return ExtensionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                     std::index_sequence_for<ARGS...>{});
  }
  // Third stage
  template <const ReturnAttribute &RETATTR, typename SELF, typename RETURN, typename... ARGS,
            RETURN (*FUNC)(SELF, ARGS...), std::size_t... I>
  NOBIND_INLINE Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                             std::integral_constant<RETURN (*)(SELF, ARGS...), FUNC>,
                                             std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    try {
      auto this_obj = FromJSValue<SELF>(info.This());
      // Call the FromJS constructors
      size_t idx = 0;
      std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
      CheckArgLength(env, idx, info.Length());
#ifndef NOBIND_NO_ASYNC_LOCKING
      FromJSLockGuard<SELF> this_guard{this_obj};
      [[maybe_unused]] std::tuple<FromJSLockGuard<ARGS>...> release_guards{std::get<I>(args)...};
#endif

      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        FUNC(this_obj.Get(), std::get<I>(args).Get()...);
        return env.Undefined();
        // FromJS objects are destroyed
      } else {
        // Convert and call
        RETURN result = FUNC(this_obj.Get(), std::get<I>(args).Get()...);
        // Call the ToJS constructor
        auto output = ToJS_t<RETURN, RETATTR>(env, result);
        // Convert
        return SetupNested<RETATTR>(output.Get());
        // FromJS/ToJS objects are destroyed
      }
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  // Setup nested objects
  template <const ReturnAttribute &RETATTR> NOBIND_INLINE Napi::Value SetupNested(Napi::Value returned) {
    if constexpr (RETATTR.isNested()) {
      if (returned.IsObject()) {
        // We simply attach the parent (this) as a hidden property in the nested object
        // This way the parent cannot be GCed until the nested objects is GCed
        returned.ToObject().DefineProperty(
            Napi::PropertyDescriptor::Value(NOBIND_PARENT_PROP, this->Value(), napi_default));
      }
    }
    return returned;
  }

  // Register a custom finalizer
  NOBIND_INLINE void SetFinalizer(Finalizer f) {
    NOBIND_ASSERT(!finalizer_);
    finalizer_ = f;
  }

  // To look up the class constructor in the per-instance data
  static size_t class_idx;
  // For TypeScript
  static std::string name;
  // The class constructors
  static std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> cons;
  // The underlying C++ object
  CLASS *self;
  // Should we destroy it in the destructor
  bool owned;
  // A custom finalizer to be called when destroying
  Finalizer finalizer_;
#ifndef NOBIND_NO_ASYNC_LOCKING
  // The async reentrancy lock
  std::mutex async_lock;
#endif
};

template <typename CLASS> size_t NoObjectWrap<CLASS>::class_idx = 0;
template <typename CLASS> std::string NoObjectWrap<CLASS>::name = NOBIND_NAME_NOT_INITIALIZED;
template <typename CLASS>
std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> NoObjectWrap<CLASS>::cons;

#ifdef NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER
template <typename CLASS> NoObjectWrap<CLASS>::~NoObjectWrap() { assert(self == nullptr); }

template <typename CLASS> void NoObjectWrap<CLASS>::Finalize(Napi::BasicEnv env) {
  NOBIND_VERBOSE_TYPE(OBJECT, CLASS, self, "synchronous (basic finalizer) delete [owned=%s]\n",
                      owned ? "true" : "false");
#else
template <typename CLASS> NoObjectWrap<CLASS>::~NoObjectWrap() {
  Napi::Env env{this->Env()};
  NOBIND_VERBOSE_TYPE(OBJECT, CLASS, self, "asynchronous delete (no basic finalizer) [owned=%s]\n",
                      owned ? "true" : "false");
#endif
#ifndef NOBIND_NO_OBJECT_STORE
  auto instance = env.GetInstanceData<BaseEnvInstanceData>();
  if (instance->_Nobind_object_store != nullptr) {
    instance->_Nobind_object_store->Expire(class_idx, self, this->Value());
  } else {
    // Finalizers seem to run after the environment cleanup hook
    // Two questions: how and why?
    NOBIND_VERBOSE(STORE, "ObjectStore has already been finalized\n");
  }
#endif

  if (finalizer_) {
    NOBIND_VERBOSE_TYPE(OBJECT, CLASS, self, "running custom finalizer\n");
    finalizer_(env, self);
  } else if (owned && self != nullptr) {
    if constexpr (!std::is_abstract_v<CLASS> && std::is_destructible_v<CLASS>) {
      delete self;
      Napi::MemoryManagement::AdjustExternalMemory(env, -static_cast<int64_t>(sizeof(CLASS)));
    } else {
      // Abstract classes cannot be deleted
      std::cerr << "Cannot delete an object of abstract or non destructible class "s + name << std::endl;
      std::terminate();
    }
  }
  self = nullptr;
}

// A constructor can be called in two ways:
// * From JS with JS arguments -> it must construct the underlying object
// * From C++ with a Napi::External<> pointer -> it must construct a proxy for this object
template <typename CLASS>
NoObjectWrap<CLASS>::NoObjectWrap(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NoObjectWrap<CLASS>>(info), finalizer_()
#ifndef NOBIND_NO_ASYNC_LOCKING
      ,
      async_lock{}
#endif
{
  Napi::Env env{info.Env()};

  if (info.Length() == 2 && info[0].IsExternal()) {
    // From C++
    owned = info[1].ToBoolean().Value();
    self = info[0].As<Napi::External<CLASS>>().Data();
    NOBIND_VERBOSE_TYPE(OBJECT, CLASS, self, "create wrapper for C++ object [owned=%s]\n", owned ? "true" : "false");
    return;
  }
  // From JS
  owned = true;
  if constexpr (std::is_abstract_v<CLASS> || !std::is_destructible_v<CLASS>) {
    throw Napi::TypeError::New(env, "Cannot create an object of abstract or non destructible class "s + name);
  }
  if (cons.size() > info.Length() && cons[info.Length()].size() > 0) {
#ifndef NOBIND_NO_OBJECT_STORE
    auto instance = env.GetInstanceData<BaseEnvInstanceData>();
#endif
    std::vector<std::string> errors;
    for (auto ctor : cons[info.Length()]) {
      try {
        (this->*ctor)(info);
#ifndef NOBIND_NO_OBJECT_STORE
        instance->_Nobind_object_store->Put(class_idx, self, this->Value());
        NOBIND_VERBOSE_TYPE(OBJECT, CLASS, self, "create new JS object with C++ object\n");
#endif
        Napi::MemoryManagement::AdjustExternalMemory(env, sizeof(CLASS));
        return;
      } catch (const Napi::Error &e) {
        // If there is only one constructor for the given number of arguments,
        // throw the original construction error, instead of a generic error
        // saying that all constructors with X arguments have been tried and none work
        if (cons[info.Length()].size() == 1) {
          std::rethrow_exception(std::current_exception());
        }
        // If not concatenate all the errors
        errors.push_back(e.what());
      } catch (const std::exception &e) {
        // Same as above
        if (cons[info.Length()].size() == 1) {
          throw Napi::TypeError::New(env, e.what());
        }
        errors.push_back(e.what());
      }
    }
    throw Napi::TypeError::New(
        env, "All constructors with "s + std::to_string(info.Length()) + " arguments tried: ["s +
                 std::accumulate(errors.begin() + 1, errors.end(), *errors.begin(),
                                 [](const std::string s1, const std::string s2) { return s1 + ", " + s2; }) +
                 "]"s);
  }
  throw Napi::TypeError::New(env, "No constructor with "s + std::to_string(info.Length()) + " arguments found");
} // namespace Nobind

template <typename CLASS>
Napi::Function
NoObjectWrap<CLASS>::GetClass(Napi::Env env, const char *name,
                              const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &properties) {
  return Napi::ObjectWrap<NoObjectWrap<CLASS>>::DefineClass(env, name, properties, nullptr);
}

template <typename CLASS>
template <bool OWNED>
NOBIND_INLINE Napi::Value NoObjectWrap<CLASS>::New(Napi::Env env, CLASS *obj, Finalizer finalizer) {
  auto instance = env.GetInstanceData<BaseEnvInstanceData>();
#ifndef NOBIND_NO_OBJECT_STORE
  Napi::Value stored = instance->_Nobind_object_store->Get(class_idx, obj);
  if (!stored.IsEmpty())
    return stored;
#endif

  napi_value ext = Napi::External<CLASS>::New(env, obj);
  napi_value own = Napi::Boolean::New(env, OWNED);
  Napi::Value r = instance->_Nobind_cons[class_idx].New({ext, own});

  if constexpr (OWNED) {
    Napi::MemoryManagement::AdjustExternalMemory(env, sizeof(CLASS));
  }

  if (finalizer) {
    NoObjectWrap<CLASS> *wrapper = NoObjectWrap<CLASS>::Unwrap(r.ToObject());
    wrapper->finalizer_ = finalizer;
  }

#ifndef NOBIND_NO_OBJECT_STORE
  instance->_Nobind_object_store->Put(class_idx, obj, r);
#endif
  return r;
}

template <typename CLASS>
template <bool OWNED>
NOBIND_INLINE Napi::Value NoObjectWrap<CLASS>::New(Napi::Env env, const CLASS *obj) {
  auto instance = env.GetInstanceData<BaseEnvInstanceData>();
#ifndef NOBIND_NO_OBJECT_STORE
  Napi::Value stored = instance->_Nobind_object_store->Get(class_idx, obj);
  if (!stored.IsEmpty())
    return stored;
#endif

  static_assert(OWNED == false, "Cannot create an owned object from a const object, use Nobind::ReturnShared");
  napi_value ext = Napi::External<CLASS>::New(env, const_cast<CLASS *>(obj));
  napi_value own = Napi::Boolean::New(env, false);
  Napi::Value r = instance->_Nobind_cons[class_idx].New({ext, own});

  if constexpr (OWNED) {
    Napi::MemoryManagement::AdjustExternalMemory(env, sizeof(CLASS));
  }

#ifndef NOBIND_NO_OBJECT_STORE
  instance->_Nobind_object_store->Put(class_idx, obj, r);
#endif
  return r;
}

template <typename CLASS> NOBIND_INLINE void NoObjectWrap<CLASS>::CheckInstance(Napi::Value val) {
  Napi::Env env(val.Env());
  if (!val.IsObject()) {
    throw Napi::TypeError::New(env, "Expected an object");
  }
  Napi::Object obj = val.ToObject();
  auto instance = env.GetInstanceData<BaseEnvInstanceData>();
  if (name == NOBIND_NAME_NOT_INITIALIZED || !obj.InstanceOf(instance->_Nobind_cons[class_idx].Value())) {
    throw Napi::TypeError::New(env, "Expected a "s +
                                        (name != NOBIND_NAME_NOT_INITIALIZED ? name : "<unknown to nobind17 class>"s));
  }
}

template <typename CLASS> NOBIND_INLINE CLASS *NoObjectWrap<CLASS>::Get() { return self; }

#ifndef NOBIND_NO_ASYNC_LOCKING
template <typename CLASS> NOBIND_INLINE void NoObjectWrap<CLASS>::Lock() NOBIND_NOEXCEPT {
  NOBIND_VERBOSE_TYPE(LOCK, CLASS, self, "Locking\n");
#if defined(NOBIND_THROW_ON_EVENT_LOOP_BLOCK) || defined(NOBIND_WARN_ON_EVENT_LOOP_BLOCK)
  Napi::Env env = this->Env();
  auto instance = env.GetInstanceData<BaseEnvInstanceData>();
  if (instance->_Nobind_js_thread == std::this_thread::get_id()) {
    bool acquired = async_lock.try_lock();
    if (acquired) {
      NOBIND_VERBOSE_TYPE(LOCK, CLASS, self, "Locked on the main thread w/o contention\n");
      return;
    } else {
#ifdef DEBUG
      std::string type = NobindDebugInstance::Demangle<CLASS>();
#else
      static const std::string type = "Enable DEBUG mode to see the object type"s;
#endif
      std::ostringstream this_ptr;
      this_ptr << std::hex << this;
      std::string msg = "Will have to block the event loop for ["s + type + "] "s + this_ptr.str() +
                        ", object is locked by a background async thread.";
      auto err = Napi::Error::New(env, msg);
#ifdef NOBIND_THROW_ON_EVENT_LOOP_BLOCK
      throw err;
#endif
#ifdef NOBIND_WARN_ON_EVENT_LOOP_BLOCK
      Napi::Object err_js = err.Value().ToObject();
      Napi::Object stack_js = err_js.Get("stack").ToObject();
      Napi::String output = stack_js.Get("toString").As<Napi::Function>().Call(stack_js, 0, nullptr).ToString();
      std::cerr << output.Utf8Value() << std::endl;
#endif
    }
  }
#endif
  async_lock.lock();
  NOBIND_VERBOSE_TYPE(LOCK, CLASS, self, "Locked\n");
}
template <typename CLASS> NOBIND_INLINE void NoObjectWrap<CLASS>::Unlock() NOBIND_NOEXCEPT {
  NOBIND_VERBOSE_TYPE(LOCK, CLASS, self, "Unlocking\n");
  async_lock.unlock();
}
#endif

// API class for defining a class binding
template <typename CLASS, typename BASE, typename... INTERFACES> class ClassDefinition {
  const char *name_;
  Napi::Env env_;
  Napi::Object exports_;
  std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> properties;
  std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> constructors;
  size_t class_idx_;
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
  std::string class_typescript_types_, &global_typescript_types_;
#endif

public:
  // Instance class method
  template <auto MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  std::enable_if_t<std::is_member_function_pointer_v<decltype(MEMBER)>, ClassDefinition &> def(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper;

    if constexpr (RET.isAsync()) {
      wrapper = &NoObjectWrap<CLASS>::template MethodWrapperAsync<RET, MEMBER>;
    } else {
      wrapper = &NoObjectWrap<CLASS>::template MethodWrapper<RET, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = MethodSignature<RET, MEMBER>(name, "  ");
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

  // Instance class method / sync+async shortcut
  template <auto MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  std::enable_if_t<std::is_member_function_pointer_v<decltype(MEMBER)>, ClassDefinition &> def(NAME name_sync,
                                                                                               NAME name_async) {
    static_assert(!RET.isAsync(), "Do not specify async with the duplex definition");
    def<MEMBER, RET>(name_sync);
    def<MEMBER, RetWithAsync<RET>>(name_async);
    return *this;
  }

  // Instance class getter/setter
  template <auto CLASS::*MEMBER, const PropertyAttribute &PROP = ReadWrite, typename NAME = const char *>
  std::enable_if_t<std::is_member_object_pointer_v<decltype(MEMBER)>, ClassDefinition &> def(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceGetterCallback getter =
        &NoObjectWrap<CLASS>::template GetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
    typename NoObjectWrap<CLASS>::InstanceSetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &NoObjectWrap<CLASS>::template SetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceAccessor(name, getter, setter));

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = PropertySignature<PROP, decltype(getMemberPointerType(MEMBER))>(name, "  ");
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

  // Static class method
  template <auto *MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>, ClassDefinition &> def(NAME name) {
    Napi::Function::Callback wrapper;
    if constexpr (RET.isAsync()) {
      wrapper = &FunctionWrapperAsync<RET, MEMBER>;
    } else {
      wrapper = &FunctionWrapper<RET, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::StaticMethod(name, wrapper));

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = FunctionSignature<RET, MEMBER>(name, "  static ");
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

  // Static class method / sync+async shortcut
  template <auto *MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>, ClassDefinition &>
  def(NAME name_sync, NAME name_async) {
    static_assert(!RET.isAsync(), "Do not specify async with the duplex definition");
    def<MEMBER, RET>(name_sync);
    def<MEMBER, RetWithAsync<RET>>(name_async);
    return *this;
  }

  // Static class getter/setter
  template <auto *MEMBER, const PropertyAttribute &PROP = ReadWrite, typename NAME = const char *>
  std::enable_if_t<!std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>, ClassDefinition &> def(NAME name) {
    typename NoObjectWrap<CLASS>::StaticGetterCallback getter =
        &GetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    typename NoObjectWrap<CLASS>::StaticSetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &NoObjectWrap<CLASS>::template StaticSetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::StaticAccessor(name, getter, setter));

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = PropertySignature<PROP, std::remove_pointer_t<decltype(MEMBER)>>(name, "  static ");
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

  // Class extension
  template <auto *FUNC, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  ClassDefinition &ext(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper;
    static_assert(!RET.isAsync(), "Asynchronous class extensions are not supported, use a global function helper");

    wrapper = &NoObjectWrap<CLASS>::template ExtensionWrapper<RET, FUNC>;
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = ExtensionSignature<RET, FUNC>(name, "  ");
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

  template <typename... ARGS> ClassDefinition &cons() {
    typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback wrapper =
        &NoObjectWrap<CLASS>::template ConsWrapper<ARGS...>;
    if (constructors.size() <= sizeof...(ARGS) + 1)
      constructors.resize(sizeof...(ARGS) + 1);
    constructors[sizeof...(ARGS)].push_back(wrapper);

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
    std::string typescript_types = "  " + ConstructorSignature<ARGS...>();
    class_typescript_types_ += typescript_types;
#endif

    return *this;
  }

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
  // Custom TypeScript fragment
  ClassDefinition &typescript_fragment(const char *fragment) {
    class_typescript_types_ += fragment;
    return *this;
  }
  ClassDefinition &typescript_fragment(const std::string &fragment) {
    class_typescript_types_ += fragment;
    return *this;
  }
#endif

  explicit ClassDefinition(const char *name, Napi::Env env, Napi::Object exports, size_t class_idx
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
                           ,
                           std::string &global_typescript_types
#endif
                           )
      : name_(name), env_(env), exports_(exports), properties(), constructors(), class_idx_(class_idx)
#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
        ,
        class_typescript_types_(""), global_typescript_types_(global_typescript_types)
#endif
  {
    NoObjectWrap<CLASS>::Declare(name);
  }

  ~ClassDefinition() noexcept(false) {
    Napi::Function ctor = NoObjectWrap<CLASS>::GetClass(env_, name_, properties);
    auto instance = env_.GetInstanceData<BaseEnvInstanceData>();
    NoObjectWrap<CLASS>::Configure(constructors, class_idx_);
    instance->_Nobind_cons.emplace(instance->_Nobind_cons.begin() + class_idx_, Napi::Persistent(ctor));
    exports_.Set(name_, ctor);

    if constexpr (!std::is_void_v<BASE>) {
      auto base_constructor = exports_.Get(NoObjectWrap<BASE>::GetName());
      if (base_constructor.IsUndefined()) {
        throw Napi::Error::New(env_,
                               "Base class "s + NoObjectWrap<BASE>::GetName() + " not found, is the class defined?"s);
      }
      auto base_prototype = base_constructor.ToObject().Get("prototype");
      ctor.ToObject().Get("prototype").ToObject().Set("__proto__", base_prototype);
    }

#ifndef NOBIND_NO_TYPESCRIPT_GENERATOR
#ifdef NOBIND_TYPESCRIPT_LOCAL_DEFINITIONS
    exports_.Get(name_).ToObject().DefineProperty(Napi::PropertyDescriptor::Value(
        NOBIND_TYPESCRIPT_PROP, Napi::String::New(env_, class_typescript_types_), napi_default));
#endif
    global_typescript_types_ += "export ";
    if constexpr (std::is_abstract_v<CLASS> || !std::is_destructible_v<CLASS>) {
      global_typescript_types_ += "abstract ";
    } else if (constructors.size() == 0) {
      global_typescript_types_ += "abstract ";
    }
    global_typescript_types_ += "class "s + name_;
    if constexpr (!std::is_void_v<BASE>)
      global_typescript_types_ += " extends "s + NoObjectWrap<BASE>::GetName();
    global_typescript_types_ += FromTSTInterfaces<INTERFACES...>();
    global_typescript_types_ += " { \n"s;
    global_typescript_types_ += class_typescript_types_;
    global_typescript_types_ += "}\n"s;
#endif
  }
};

namespace Typemap {

// Generic object reference typemap
template <typename T> class FromJS<T &> {
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
  T *val_;
  OBJCLASS *wrapper_;
  Napi::ObjectReference persistent_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a FromJS typemap");
    OBJCLASS::CheckInstance(val);
    wrapper_ = OBJCLASS::Unwrap(val.ToObject());
    val_ = wrapper_->Get();
    persistent_ = Napi::Persistent(val.ToObject());
  }
  NOBIND_INLINE T &Get() { return *val_; }

#ifndef NOBIND_NO_ASYNC_LOCKING
  NOBIND_INLINE void Lock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, val_, "FromJS & Lock\n");
    if (wrapper_)
      wrapper_->Lock();
  }
  NOBIND_INLINE void Unlock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, val_, "FromJS & Unlock\n");
    if (wrapper_)
      wrapper_->Unlock();
  }
#endif

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T &, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, T &val) : env_(env), val_(&val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a ToJS typemap");
  }
  // C++ returned a reference, we consider this function to return a static object
  // By default, the JS proxy will not own this object
  NOBIND_INLINE Napi::Value Get() {
    if constexpr (RETATTR.isCopy()) {
      return OBJCLASS::template New<true>(env_, new T{*val_});
    }
    return OBJCLASS::template New<RETATTR.ShouldOwn<false>()>(env_, val_);
  }

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

// Generic object pointer typemap
template <typename T> class FromJS<T *> {
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
  T *val_;
  OBJCLASS *wrapper_;
  Napi::ObjectReference persistent_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a FromJS typemap");
    OBJCLASS::CheckInstance(val);
    wrapper_ = OBJCLASS::Unwrap(val.ToObject());
    val_ = wrapper_->Get();
    persistent_ = Napi::Persistent(val.ToObject());
  }
  NOBIND_INLINE T *Get() { return val_; }

#ifndef NOBIND_NO_ASYNC_LOCKING
  NOBIND_INLINE void Lock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, val_, "FromJS * Lock\n");
    if (wrapper_)
      wrapper_->Lock();
  }
  NOBIND_INLINE void Unlock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, val_, "FromJS * Unlock\n");
    if (wrapper_)
      wrapper_->Unlock();
  }
#endif

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T *, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, T *val) : env_(env), val_(val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a ToJS typemap");
  }
  // We consider this to be a factory function, it has returned a pointer
  // By default, the JS proxy will own this object
  NOBIND_INLINE Napi::Value Get() {
    if constexpr (!RETATTR.isReturnNullThrow()) {
      if (val_ == nullptr) {
        return env_.Null();
      }
    } else {
      if (val_ == nullptr) {
        throw Napi::Error::New(env_, "Returned nullptr");
      }
    }
    if constexpr (RETATTR.isCopy()) {
      return OBJCLASS::template New<true>(env_, new T{*val_});
    }
    return OBJCLASS::template New<RETATTR.ShouldOwn<true>()>(env_, val_);
  }

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

// Generic stack-allocated object typemaps
template <typename T> class FromJS {
  T *object_;
  NoObjectWrap<T> *wrapper_;
  Napi::ObjectReference persistent_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a FromJS typemap");
    // C++ asks for a regular stack-allocated object
    NoObjectWrap<T>::CheckInstance(val);
    wrapper_ = NoObjectWrap<T>::Unwrap(val.ToObject());
    object_ = wrapper_->Get();
    persistent_ = Napi::Persistent(val.ToObject());
  }

  // will return a copy by value
  NOBIND_INLINE T Get() { return *object_; }

#ifndef NOBIND_NO_ASYNC_LOCKING
  NOBIND_INLINE void Lock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, object_, "FromJS Lock\n");
    if (wrapper_)
      wrapper_->Lock();
  }
  NOBIND_INLINE void Unlock() NOBIND_NOEXCEPT {
    NOBIND_VERBOSE_TYPE(LOCK, T, object_, "FromJS Unlock\n");
    if (wrapper_)
      wrapper_->Unlock();
  }
#endif

  static const size_t Inputs = 1;

  static const std::string &TSType() { return NoObjectWrap<T>::GetName(); };
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS {
  Napi::Env env_;
  T *object;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, T &val) : env_(env) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "Type does not have a ToJS typemap");
    // C++ returned regular object
    if constexpr (RETATTR.isNested()) {
      // This is a nested object from a getter (for a class member object), return a nested reference
      object = &val;
    } else {
      // This is a stack-allocated object, copy/move it to the heap
      if constexpr (std::is_copy_constructible_v<T>)
        object = new T(val);
      else
        object = new T(std::move(val));
    }
  }
  // and wrapping it in a proxy, by default JS will own this new copy
  NOBIND_INLINE Napi::Value Get() { return NoObjectWrap<T>::template New<RETATTR.ShouldOwn<true>()>(env_, object); }

  static const std::string &TSType() { return NoObjectWrap<T>::GetName(); };
};

} // namespace Typemap

} // namespace Nobind
