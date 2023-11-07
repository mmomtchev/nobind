#pragma once
#include <assert.h>
#include <functional>
#include <napi.h>
#include <tuple>
#include <type_traits>

#include <notypes.h>
#include <nofunction.h>

namespace Nobind {

struct EnvInstanceData {
  // Per-environment constructors for all proxied types
  std::vector<Napi::FunctionReference> cons;
};

// The JS proxy object type
template <typename CLASS> class NoObjectWrap : public Napi::ObjectWrap<NoObjectWrap<CLASS>> {
  template <typename T> friend class Typemap::FromJS;
  template <typename T, const ReturnAttribute &RETATTR> friend class Typemap::ToJS;

public:
  // JS convention constructor
  NoObjectWrap(const Napi::CallbackInfo &);
  // C++ convention constructor
  static Napi::Value New(Napi::Env, CLASS *, bool);
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
  template <const ReturnAttribute &RET = ReturnDefault, auto CLASS::*FUNC>
  Napi::Value MethodWrapper(const Napi::CallbackInfo &info) {
    return MethodWrapper<RET>(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  template <typename T, T CLASS::*MEMBER> Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return *ToJS<T, ReturnDefault>(env, self->*MEMBER);
  }

  template <typename T, T CLASS::*MEMBER> void SetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    self->*MEMBER = *FromJS<T>(val);
  }

  template <typename T, T *MEMBER> static Napi::Value StaticGetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return *ToJS<T, ReturnDefault>(env, *MEMBER);
  }

  template <typename T, T *MEMBER>
  static void StaticSetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    *MEMBER = *FromJS<T>(val);
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
  template <const ReturnAttribute &RETATTR = ReturnDefault, typename RETURN, typename... ARGS,
            RETURN (CLASS::*FUNC)(ARGS...)>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (CLASS::*)(ARGS...), FUNC>) {
    return MethodWrapper<RETATTR>(info, std::integral_constant<decltype(FUNC), FUNC>{},
                                  std::index_sequence_for<ARGS...>{});
  }
  template <const ReturnAttribute &RETATTR = ReturnDefault, typename RETURN, typename... ARGS,
            RETURN (CLASS::*FUNC)(ARGS...), std::size_t... I>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (CLASS::*)(ARGS...), FUNC>,
                                   std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    CheckArgLength<ARGS...>(env, info.Length());
    try {
      if constexpr (sizeof...(ARGS) > 0) {
        // Call the FromJS constructors
        std::tuple<Nobind::Typemap::FromJS<ARGS>...> args(Nobind::FromJS<ARGS>(info[I])...);
        if constexpr (std::is_void_v<RETURN>) {
          // Convert and call
          (self->*FUNC)(*std::get<I>(args)...);
          return env.Undefined();
          // FromJS objects are destroyed
        } else {
          // Convert and call
          RETURN result = (self->*FUNC)(*std::get<I>(args)...);
          // Call the ToJS constructor
          auto output = Nobind::Typemap::ToJS<RETURN, RETATTR>(env, result);
          // Convert
          return *output;
          // FromJS/ToJS objects are destroyed
        }
      } else {
        if constexpr (std::is_void_v<RETURN>) {
          // Call
          (self->*FUNC)();
          return env.Undefined();
        } else {
          // Call
          RETURN result = (self->*FUNC)();
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

  // The constructor wrapper implementation
  template <typename... ARGS, std::size_t... I>
  inline void ConsWrapper(const Napi::CallbackInfo &info, std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    CheckArgLength<ARGS...>(env, info.Length());
    if constexpr (sizeof...(ARGS) > 0) {
      self = new CLASS(*Nobind::FromJS<ARGS>(info[I])...);
    } else {
      self = new CLASS;
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

template <typename CLASS> inline Napi::Value NoObjectWrap<CLASS>::New(Napi::Env env, CLASS *obj, bool ownership) {
  napi_value ext = Napi::External<CLASS>::New(env, obj);
  napi_value own = Napi::Boolean::New(env, ownership);
  auto instance = env.GetInstanceData<EnvInstanceData>();
  Napi::Value r = instance->cons[class_idx].New({ext, own});
  return r;
}

template <typename CLASS> inline CLASS *NoObjectWrap<CLASS>::CheckUnwrap(Napi::Value val) {
  Napi::Env env(val.Env());
  if (!val.IsObject()) {
    throw Napi::TypeError::New(env, "Not an object");
  }
  Napi::Object obj = val.ToObject();
  auto instance = env.GetInstanceData<EnvInstanceData>();
  if (!obj.InstanceOf(instance->cons[class_idx].Value())) {
    throw Napi::TypeError::New(env, "Not a " + name);
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
  template <auto CLASS::*MEMBER, const ReturnAttribute &RET = ReturnDefault,
            typename = std::enable_if_t<std::is_member_function_pointer_v<decltype(MEMBER)>>>
  ClassDefinition &def(const char *name) {
    typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper =
        &NoObjectWrap<CLASS>::template MethodWrapper<RET, MEMBER>;
    properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));

    return *this;
  }

  // Instance class getter/setter
  template <auto CLASS::*MEMBER, const PropertyAttribute &PROP = ReadWrite,
            typename = std::enable_if_t<std::is_member_object_pointer_v<decltype(MEMBER)>>>
  ClassDefinition &def(const char *name) {
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
  template <auto *MEMBER, const ReturnAttribute &RET = ReturnDefault,
            typename = std::enable_if_t<std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>>>
  ClassDefinition &def(const char *name) {
    typename NoObjectWrap<CLASS>::StaticMethodCallback wrapper =
        &FunctionWrapper<RET, MEMBER>;
    properties.emplace_back(NoObjectWrap<CLASS>::StaticMethod(name, wrapper));
    return *this;
  }

  // Static class getter/setter
  template <auto *MEMBER, const PropertyAttribute &PROP = ReadWrite,
            typename = std::enable_if_t<!std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>>>
  ClassDefinition &def(const char *name) {
    typename NoObjectWrap<CLASS>::StaticGetterCallback getter =
        &NoObjectWrap<CLASS>::template StaticGetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    typename NoObjectWrap<CLASS>::StaticSetterCallback setter = nullptr;
    if constexpr (!PROP.isReadOnly()) {
      setter = &NoObjectWrap<CLASS>::template StaticSetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
    }
    properties.emplace_back(NoObjectWrap<CLASS>::StaticAccessor(name, getter, setter));
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

public:
  inline explicit FromJS(Napi::Value val) {
    if constexpr (std::is_object_v<T>) {
      using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
      Napi::Env env = val.Env();
      val_ = OBJCLASS::CheckUnwrap(val);
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T &operator*() { return *val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T &, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  inline explicit ToJS(Napi::Env env, T &val) : env_(env), val_(&val) {
    if constexpr (std::is_object_v<T>) {
      return;
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // C++ returned a reference, we consider this function to return a static object
  // By default, the JS proxy will not own this object
  inline Napi::Value operator*() { return OBJCLASS::New(env_, val_, RETATTR.ShouldOwn<false>()); }
};

// Generic object pointer typemap
template <typename T> class FromJS<T *> {
  T *val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if constexpr (std::is_object_v<T>) {
      using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
      Napi::Env env = val.Env();
      val_ = OBJCLASS::CheckUnwrap(val);
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T *operator*() { return val_; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS<T *, RETATTR> {
  Napi::Env env_;
  T *val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  inline explicit ToJS(Napi::Env env, T *val) : env_(env), val_(val) {
    if constexpr (std::is_object_v<T>) {
      return;
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // We consider this to be a factory function, it has returned a pointer
  // By default, the JS proxy will own this object
  inline Napi::Value operator*() { return OBJCLASS::New(env_, val_, RETATTR.ShouldOwn<true>()); }
};

// Generic stack-allocated object typemaps
template <typename T> class FromJS {
  T *object;

public:
  inline explicit FromJS(Napi::Value val) {
    if constexpr (std::is_object_v<T>) {
      // C++ asks for a regular stack-allocated object
      object = NoObjectWrap<T>::CheckUnwrap(val);
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  // will return a copy by value
  inline T operator*() { return *object; }
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS {
  Napi::Env env_;
  T *object;

public:
  inline explicit ToJS(Napi::Env env, T val) : env_(env) {
    if constexpr (std::is_object_v<T>) {
      // C++ returned regular stack-allocated object, import to JS by copying to the heap
      object = new T(val);
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a ToJS typemap");
    }
  }
  // and wrapping it in a proxy, by default JS will own this new copy
  inline Napi::Value operator*() { return NoObjectWrap<T>::New(env_, object, RETATTR.ShouldOwn<true>()); }
};

} // namespace Typemap

} // namespace Nobind
