#pragma once
#include <assert.h>
#include <functional>
#include <napi.h>
#include <tuple>
#include <type_traits>

#include <nofunction.h>
#include <notypes.h>

namespace Nobind {

struct EnvInstanceData {
  // Per-environment constructors for all proxied types
  std::vector<Napi::FunctionReference> cons;
};

// The JS proxy object type
template <typename CLASS> class NoObjectWrap : public Napi::ObjectWrap<NoObjectWrap<CLASS>> {
  template <typename T> friend class Typemap::FromJS;
  template <typename T, const ReturnAttribute &RETATTR> friend class Typemap::ToJS;

  template <const ReturnAttribute &RETATTR, typename BASE, auto FUNC, typename RETURN, typename... ARGS>
  class MethodWrapperTasklet : public Napi::AsyncWorker {
    Napi::Env env_;
    Napi::Promise::Deferred deferred_;
    std::unique_ptr<ToJS_t<RETURN, RETATTR>> output;
    std::tuple<FromJS_t<ARGS>...> args_;
    BASE *self_;

  public:
    MethodWrapperTasklet(Napi::Env env, Napi::Promise::Deferred deferred, CLASS *self,
                         std::tuple<FromJS_t<ARGS>...> &&args)
        : AsyncWorker(env, "nobind_AsyncWorker"), env_(env), deferred_(deferred), output(), args_(std::move(args)),
          self_(static_cast<BASE *>(self)) {}

    template <std::size_t... I> void ExecuteImpl(std::index_sequence<I...>) {
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
          auto result = output->Get();
          deferred_.Resolve(result);
        } catch (const std::exception &e) {
          deferred_.Reject(Napi::String::New(env_, e.what()));
        }
      }
    }

    virtual void OnError(const Napi::Error &e) override { deferred_.Reject(e.Value()); }
  };

public:
  // JS convention constructor
  NoObjectWrap(const Napi::CallbackInfo &);
  // C++ convention constructors
  template <bool OWNED> static Napi::Value New(Napi::Env, CLASS *);
  template <bool OWNED> static Napi::Value New(Napi::Env, const CLASS *);
  virtual ~NoObjectWrap();
  static Napi::Function GetClass(Napi::Env, const char *,
                                 const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &);

  // Check types and extract the proxied C++ object
  static CLASS *CheckUnwrap(Napi::Value);

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

  template <const ReturnAttribute &RET = ReturnDefault, auto FUNC>
  Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info) {
    return ExtensionWrapper<RET>(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  template <typename T, T CLASS::*MEMBER> Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return ToJS<T, ReturnDefault>(env, self->*MEMBER).Get();
  }

  template <typename T, T CLASS::*MEMBER> void SetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    self->*MEMBER = FromJSValue<T>(val).Get();
  }

  template <typename T, T *MEMBER>
  static void StaticSetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    *MEMBER = FromJSValue<T>(val).Get();
  }

  static void
  Configure(const std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> &constructors,
            size_t idx, const char *jsname) {
    // (class_idx == 0) - first module initialization
    // (class_idx == idx) - subsequent initialization (worker_thread)
    assert(class_idx == 0 || class_idx == idx);
    class_idx = idx;
    name = jsname;
    cons = constructors;
  }

private:
  // The two remaining functions of the member method wrapper trio
  // The first (second of the three) has 4 possibles signatures:
  // - regular, const, noexcept and const noexcept
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...)>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (BASE::*)(ARGS...), FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (BASE::*)(ARGS...) const, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) noexcept>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (BASE::*)(ARGS...) noexcept, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const noexcept>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (BASE::*)(ARGS...) const noexcept, FUNC>) {
    return MethodWrapper<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  // The last one of the trio
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, auto FUNC, typename... ARGS,
            std::size_t... I>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    size_t idx = 0;
    try {
      // Call the FromJS constructors
      std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
      CheckArgLength(env, idx, info.Length());
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
        return output.Get();
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
  inline Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (BASE::*)(ARGS...), FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const>
  inline Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (BASE::*)(ARGS...) const, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) noexcept>
  inline Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (BASE::*)(ARGS...) noexcept, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
            RETURN (BASE::*FUNC)(ARGS...) const noexcept>
  inline Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info,
                                        std::integral_constant<RETURN (BASE::*)(ARGS...) const noexcept, FUNC>) {
    return MethodWrapperAsync<RETATTR, BASE, RETURN, FUNC, ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, auto FUNC, typename... ARGS,
            std::size_t... I>
  inline Napi::Value MethodWrapperAsync(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    try {
      size_t idx = 0;
      // Alas, std::forward_as_tuple does not guarantee
      // the evaluation order of its arguments, only *braced-init-list* lists do
      // https://en.cppreference.com/w/cpp/language/list_initialization
      auto tasklet = new MethodWrapperTasklet<RETATTR, BASE, FUNC, RETURN, ARGS...>(env, deferred, self,
                                                                                    {FromJSArgs<ARGS>(info, idx)...});
      CheckArgLength(env, idx, info.Length());

      tasklet->Queue();
    } catch (const std::exception &e) {
      deferred.Reject(Napi::Error::New(env, e.what()).Value());
    }
    return deferred.Promise();
  }

  // The constructor wrapper implementation
  template <typename... ARGS, std::size_t... I>
  inline void ConsWrapper(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    // Call the FromJS constructors
    size_t idx = 0;
    std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
    CheckArgLength(env, idx, info.Length());

    // Convert and call
    self = new CLASS(std::get<I>(args).Get()...);
  }

  template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(CLASS &, ARGS...)>
  inline Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                      std::integral_constant<RETURN (*)(CLASS &, ARGS...), FUNC>) {
    return ExtensionWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                     std::index_sequence_for<ARGS...>{});
  }
  // The extension wrapper, it adds an additional first argument by converting info.This()
  template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(CLASS &, ARGS...),
            std::size_t... I>
  inline Napi::Value ExtensionWrapper(const Napi::CallbackInfo &info,
                                      std::integral_constant<RETURN (*)(CLASS &, ARGS...), FUNC>,
                                      std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    try {
      auto thisObj = FromJSValue<CLASS &>(info.This());
      // Call the FromJS constructors
      size_t idx = 0;
      std::tuple<FromJS_t<ARGS>...> args{FromJSArgs<ARGS>(info, idx)...};
      CheckArgLength(env, idx, info.Length());
      if constexpr (std::is_void_v<RETURN>) {
        // Convert and call
        FUNC(thisObj.Get(), std::get<I>(args).Get()...);
        return env.Undefined();
        // FromJS objects are destroyed
      } else {
        // Convert and call
        RETURN result = FUNC(thisObj.Get(), std::get<I>(args).Get()...);
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

  // To look up the class constructor in the per-instance data
  static size_t class_idx;
  // Mainly for debug purposes
  static std::string name;
  // The class constructors
  static std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> cons;
  // The underlying C++ object
  CLASS *self;
  // Should we destroy it in the destructor
  bool owned;
};

template <typename CLASS> size_t NoObjectWrap<CLASS>::class_idx = 0;
template <typename CLASS> std::string NoObjectWrap<CLASS>::name;
template <typename CLASS>
std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> NoObjectWrap<CLASS>::cons;

template <typename CLASS> NoObjectWrap<CLASS>::~NoObjectWrap() {
  if (owned && self != nullptr)
    delete self;
}

// A constructor can be called in two ways:
// * From JS with JS arguments -> it must construct the underlying object
// * From C++ with a Napi::External<> pointer -> it must construct a proxy for this object
template <typename CLASS>
NoObjectWrap<CLASS>::NoObjectWrap(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NoObjectWrap<CLASS>>(info) {
  // Napi::Env env = info.Env();
  if (info.Length() == 2 && info[0].IsExternal()) {
    // From C++
    owned = info[1].ToBoolean().Value();
    self = info[0].As<Napi::External<CLASS>>().Data();
    return;
  }
  // From JS
  owned = true;
  if (cons.size() > info.Length() && cons[info.Length()].size() > 0) {
    for (auto ctor : cons[info.Length()]) {
      try {
        (this->*ctor)(info);
        return;
      } catch (...) {
      }
    }
  }
  throw Napi::TypeError::New(info.Env(),
                             "No constructor with the given " + std::to_string(info.Length()) + " arguments found");
}

template <typename CLASS>
Napi::Function
NoObjectWrap<CLASS>::GetClass(Napi::Env env, const char *name,
                              const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &properties) {
  return Napi::ObjectWrap<NoObjectWrap<CLASS>>::DefineClass(env, name, properties, nullptr);
}

template <typename CLASS> template <bool OWNED> inline Napi::Value NoObjectWrap<CLASS>::New(Napi::Env env, CLASS *obj) {
  napi_value ext = Napi::External<CLASS>::New(env, obj);
  napi_value own = Napi::Boolean::New(env, OWNED);
  auto instance = env.GetInstanceData<EnvInstanceData>();
  Napi::Value r = instance->cons[class_idx].New({ext, own});
  return r;
}

template <typename CLASS>
template <bool OWNED>
inline Napi::Value NoObjectWrap<CLASS>::New(Napi::Env env, const CLASS *obj) {
  static_assert(OWNED == false, "Cannot create an owned object from a const object, use Nobind::ReturnShared");
  napi_value ext = Napi::External<CLASS>::New(env, const_cast<CLASS *>(obj));
  napi_value own = Napi::Boolean::New(env, false);
  auto instance = env.GetInstanceData<EnvInstanceData>();
  Napi::Value r = instance->cons[class_idx].New({ext, own});
  return r;
}

template <typename CLASS> inline CLASS *NoObjectWrap<CLASS>::CheckUnwrap(Napi::Value val) {
  Napi::Env env(val.Env());
  if (!val.IsObject()) {
    throw Napi::TypeError::New(env, "Expected an object");
  }
  Napi::Object obj = val.ToObject();
  auto instance = env.GetInstanceData<EnvInstanceData>();
  if (!obj.InstanceOf(instance->cons[class_idx].Value())) {
    throw Napi::TypeError::New(env, "Expected a " + name);
  }
  return NoObjectWrap<CLASS>::Unwrap(obj)->self;
}

// API class for defining a class binding
template <class CLASS> class ClassDefinition {
  const char *name_;
  Napi::Env env_;
  Napi::Object exports_;
  std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> properties;
  std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> constructors;
  size_t class_idx_;

public:
  // Instance class method
  template <auto MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *,
            typename = std::enable_if_t<std::is_member_function_pointer_v<decltype(MEMBER)>>>
  ClassDefinition &def(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper;

    if constexpr (RET.isAsync()) {
      wrapper = &NoObjectWrap<CLASS>::template MethodWrapperAsync<RET, MEMBER>;
    } else {
      wrapper = &NoObjectWrap<CLASS>::template MethodWrapper<RET, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));

    return *this;
  }

  // Instance class getter/setter
  template <auto CLASS::*MEMBER, const PropertyAttribute &PROP = ReadWrite, typename NAME = const char *,
            typename = std::enable_if_t<std::is_member_object_pointer_v<decltype(MEMBER)>>>
  ClassDefinition &def(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceGetterCallback getter =
        &NoObjectWrap<CLASS>::template GetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
    typename NoObjectWrap<CLASS>::InstanceSetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &NoObjectWrap<CLASS>::template SetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceAccessor(name, getter, setter));
    return *this;
  }

  // Static class method
  template <auto *MEMBER, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *,
            typename = std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>>>
  ClassDefinition &def(NAME name) {
    Napi::Function::Callback wrapper;
    if constexpr (RET.isAsync()) {
      wrapper = &FunctionWrapperAsync<RET, MEMBER>;
    } else {
      wrapper = &FunctionWrapper<RET, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::StaticMethod(name, wrapper));
    return *this;
  }

  // Static class getter/setter
  template <auto *MEMBER, const PropertyAttribute &PROP = ReadWrite, typename NAME = const char *,
            typename = std::enable_if_t<!std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>>>
  ClassDefinition &def(NAME name) {
    typename NoObjectWrap<CLASS>::StaticGetterCallback getter =
        &GetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    typename NoObjectWrap<CLASS>::StaticSetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &NoObjectWrap<CLASS>::template StaticSetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::StaticAccessor(name, getter, setter));
    return *this;
  }

  // Class extension
  template <auto *FUNC, const ReturnAttribute &RET = ReturnDefault, typename NAME = const char *>
  ClassDefinition &ext(NAME name) {
    typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper;
    static_assert(!RET.isAsync(), "Asynchronous class extensions are not supported, use a global function helper");

    wrapper = &NoObjectWrap<CLASS>::template ExtensionWrapper<RET, FUNC>;
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));

    return *this;
  }

  template <typename... ARGS> ClassDefinition &cons() {
    typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback wrapper =
        &NoObjectWrap<CLASS>::template ConsWrapper<ARGS...>;
    if (constructors.size() <= sizeof...(ARGS) + 1)
      constructors.resize(sizeof...(ARGS) + 1);
    constructors[sizeof...(ARGS)].push_back(wrapper);
    return *this;
  }

  ClassDefinition(const char *name, Napi::Env env, Napi::Object exports, size_t class_idx)
      : name_(name), env_(env), exports_(exports), properties(), constructors(), class_idx_(class_idx) {}

  ~ClassDefinition() {
    Napi::Function ctor = NoObjectWrap<CLASS>::GetClass(env_, name_, properties);
    auto instance = env_.GetInstanceData<EnvInstanceData>();
    NoObjectWrap<CLASS>::Configure(constructors, class_idx_, name_);
    instance->cons.emplace(instance->cons.begin() + class_idx_, Napi::Persistent(ctor));
    exports_.Set(name_, ctor);
  }
};

namespace Typemap {

// Generic object reference typemap
template <typename T> class FromJS<T &> {
  T *val_;
  Napi::ObjectReference persistent;

public:
  inline explicit FromJS(const Napi::Value &val) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
      val_ = OBJCLASS::CheckUnwrap(val);
      persistent = Napi::Persistent(val.ToObject());
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T &Get() { return *val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T &, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  inline explicit ToJS(Napi::Env env, T &val) : env_(env), val_(&val) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      return;
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // C++ returned a reference, we consider this function to return a static object
  // By default, the JS proxy will not own this object
  inline Napi::Value Get() { return OBJCLASS::template New<RETATTR.ShouldOwn<false>()>(env_, val_); }
};

// Generic object pointer typemap
template <typename T> class FromJS<T *> {
  T *val_;
  Napi::ObjectReference persistent;

public:
  inline explicit FromJS(const Napi::Value &val) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
      val_ = OBJCLASS::CheckUnwrap(val);
      persistent = Napi::Persistent(val.ToObject());
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T *Get() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T *, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  inline explicit ToJS(Napi::Env env, T *val) : env_(env), val_(val) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      return;
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // We consider this to be a factory function, it has returned a pointer
  // By default, the JS proxy will own this object
  inline Napi::Value Get() {
    if constexpr (!RETATTR.isReturnNullThrow()) {
      if (val_ == nullptr)
        return env_.Null();
    } else {
      if (val_ == nullptr) {
        throw Napi::Error::New(env_, "Returned nullptr");
      }
    }
    return OBJCLASS::template New<RETATTR.ShouldOwn<true>()>(env_, val_);
  }
};

// Generic stack-allocated object typemaps
template <typename T> class FromJS {
  T *object;
  Napi::ObjectReference persistent;

public:
  inline explicit FromJS(const Napi::Value &val) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      // C++ asks for a regular stack-allocated object
      object = NoObjectWrap<T>::CheckUnwrap(val);
      persistent = Napi::Persistent(val.ToObject());
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  // will return a copy by value
  inline T Get() { return *object; }

  static const size_t Inputs = 1;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS {
  Napi::Env env_;
  T *object;

public:
  inline explicit ToJS(Napi::Env env, T val) : env_(env) {
    if constexpr (std::is_object_v<T> && !std::is_pod_v<T>) {
      // C++ returned regular stack-allocated object, import to JS by copying to the heap
      object = new T(val);
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // and wrapping it in a proxy, by default JS will own this new copy
  inline Napi::Value Get() { return NoObjectWrap<T>::template New<RETATTR.ShouldOwn<true>()>(env_, object); }
};

} // namespace Typemap

} // namespace Nobind
