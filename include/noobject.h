#pragma once
#include <assert.h>
#include <functional>
#include <napi.h>
#include <tuple>
#include <type_traits>

#include <notypes.h>

namespace Nobind {

struct EnvInstanceData {
  // Per-environment constructors for all proxied types
  std::vector<Napi::FunctionReference> cons;
};

// The JS proxy object type
template <typename CLASS> class NoObjectWrap : public Napi::ObjectWrap<NoObjectWrap<CLASS>> {
  template <typename T> friend class Typemap::FromJS;

public:
  NoObjectWrap(const Napi::CallbackInfo &);
  virtual ~NoObjectWrap();
  static Napi::Function GetClass(Napi::Env, const char *,
                                 const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &);

  // Constructor wrapper, these are only a pair - there are no pointers to constructors in C++
  template <typename... ARGS> void ConsWrapper(const Napi::CallbackInfo &info) {
    ConsWrapper<ARGS...>(info, std::index_sequence_for<ARGS...>{});
  }

  // The first function of the member method wrapper trio (same std::integral_constant trick)
  template <auto CLASS::*FUNC> Napi::Value MethodWrapper(const Napi::CallbackInfo &info) {
    return MethodWrapper(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  // Static member wrappers are almost identical
  template <auto *FUNC> static Napi::Value StaticMethodWrapper(const Napi::CallbackInfo &info) {
    return StaticMethodWrapper(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  template <typename T, T CLASS::*MEMBER> Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return *Typemap::ToJS<T>(env, self->*MEMBER);
  }

  template <typename T, T CLASS::*MEMBER> void SetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    self->*MEMBER = *Typemap::FromJS<T>(val);
  }

  template <typename T, T *MEMBER> static Napi::Value StaticGetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return *Typemap::ToJS<T>(env, *MEMBER);
  }

  template <typename T, T *MEMBER>
  static void StaticSetterWrapper(const Napi::CallbackInfo &info, const Napi::Value &val) {
    *MEMBER = *Typemap::FromJS<T>(val);
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
  template <typename RETURN, typename... ARGS, RETURN (CLASS::*FUNC)(ARGS...)>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (CLASS::*)(ARGS...), FUNC>) {
    return MethodWrapper(info, std::integral_constant<decltype(FUNC), FUNC>{}, std::index_sequence_for<ARGS...>{});
  }
  template <typename RETURN, typename... ARGS, RETURN (CLASS::*FUNC)(ARGS...), std::size_t... I>
  inline Napi::Value MethodWrapper(const Napi::CallbackInfo &info,
                                   std::integral_constant<RETURN (CLASS::*)(ARGS...), FUNC>,
                                   std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    CheckArgLength<ARGS...>(env, info.Length());
    try {
      if constexpr (sizeof...(ARGS) > 0) {
        if constexpr (std::is_void_v<RETURN>) {
          (self->*FUNC)(*Nobind::FromJS<ARGS>(info[I])...);
          return env.Undefined();
        } else {
          RETURN result = (self->*FUNC)(*Nobind::FromJS<ARGS>(info[I])...);
          return *Typemap::ToJS<RETURN>(env, result);
        }
      } else {
        if constexpr (std::is_void_v<RETURN>) {
          (self->*FUNC)();
          return env.Undefined();
        } else {
          RETURN result = (self->*FUNC)();
          return *Typemap::ToJS<RETURN>(env, result);
        }
      }
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  // The two remaining functions of the static member method wrapper trio
  template <typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...)>
  inline static Napi::Value StaticMethodWrapper(const Napi::CallbackInfo &info,
                                                std::integral_constant<RETURN (*)(ARGS...), FUNC>) {
    return StaticMethodWrapper(info, std::integral_constant<decltype(FUNC), FUNC>{},
                               std::index_sequence_for<ARGS...>{});
  }
  template <typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...), std::size_t... I>
  inline static Napi::Value StaticMethodWrapper(const Napi::CallbackInfo &info,
                                                std::integral_constant<RETURN (*)(ARGS...), FUNC>,
                                                std::index_sequence<I...>) {
    Napi::Env env = info.Env();

    CheckArgLength<ARGS...>(env, info.Length());
    try {
      if constexpr (sizeof...(ARGS) > 0) {
        if constexpr (std::is_void_v<RETURN>) {
          (*FUNC)(*Nobind::FromJS<ARGS>(info[I])...);
          return env.Undefined();
        } else {
          RETURN result = (*FUNC)(*Nobind::FromJS<ARGS>(info[I])...);
          return *Typemap::ToJS<RETURN>(env, result);
        }
      } else {
        if constexpr (std::is_void_v<RETURN>) {
          (*FUNC)();
          return env.Undefined();
        } else {
          RETURN result = (*FUNC)();
          return *Typemap::ToJS<RETURN>(env, result);
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
  if (info.Length() == 1 && info[0].IsExternal()) {
    // From C++
    owned = false;
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

// API class for defining a class binding
template <class CLASS> class ClassDefinition {
  const char *name_;
  Napi::Env env_;
  Napi::Object exports_;
  std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> properties;
  std::vector<std::vector<typename NoObjectWrap<CLASS>::InstanceVoidMethodCallback>> constructors;
  size_t class_idx_;

public:
  // Instance class method/getter
  template <auto CLASS::*MEMBER> ClassDefinition &def(const char *name) {
    if constexpr (std::is_member_function_pointer_v<decltype(MEMBER)>) {
      typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper =
          &NoObjectWrap<CLASS>::template MethodWrapper<MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));
    } else {
      typename NoObjectWrap<CLASS>::InstanceGetterCallback getter =
          &NoObjectWrap<CLASS>::template GetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
      typename NoObjectWrap<CLASS>::InstanceSetterCallback setter =
          &NoObjectWrap<CLASS>::template SetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::InstanceAccessor(name, getter, setter));
    }
    return *this;
  }

  // Static class method/getter
  template <auto *MEMBER> ClassDefinition &def(const char *name) {
    if constexpr (std::is_function_v<std::remove_pointer_t<decltype(MEMBER)>>) {
      typename NoObjectWrap<CLASS>::StaticMethodCallback wrapper =
          &NoObjectWrap<CLASS>::template StaticMethodWrapper<MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::StaticMethod(name, wrapper));
    } else {
      typename NoObjectWrap<CLASS>::StaticGetterCallback getter =
          &NoObjectWrap<CLASS>::template StaticGetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
      typename NoObjectWrap<CLASS>::StaticSetterCallback setter =
          &NoObjectWrap<CLASS>::template StaticSetterWrapper<std::remove_pointer_t<decltype(MEMBER)>, MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::StaticAccessor(name, getter, setter));
    }
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
  inline FromJS(Napi::Value val) {
    if constexpr (std::is_object_v<T>) {
      Napi::Env env = val.Env();
      if (val.IsObject()) {
        using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
        Napi::Object obj = val.ToObject();
        auto instance = env.GetInstanceData<EnvInstanceData>();
        size_t class_idx = OBJCLASS::class_idx;
        if (!obj.InstanceOf(instance->cons[class_idx].Value())) {
          throw Napi::TypeError::New(env, "Not a " + OBJCLASS::name);
        }
        val_ = OBJCLASS::Unwrap(obj)->self;
      } else {
        throw Napi::TypeError::New(env, "Not an object");
      }
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T &operator*() { return *val_; }
};

// Generic object pointer typemap
template <typename T> class FromJS<T *> {
  T *val_;

public:
  inline FromJS(Napi::Value val) {
    if constexpr (std::is_object_v<T>) {
      Napi::Env env = val.Env();
      if (val.IsObject()) {
        using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
        Napi::Object obj = val.ToObject();
        auto instance = env.GetInstanceData<EnvInstanceData>();
        size_t class_idx = OBJCLASS::class_idx;
        if (!obj.InstanceOf(instance->cons[class_idx].Value())) {
          throw Napi::TypeError::New(env, "Not a " + OBJCLASS::name);
        }
        val_ = OBJCLASS::Unwrap(obj)->self;
      } else {
        throw Napi::TypeError::New(env, "Not an object");
      }
    } else {
      static_assert(!std::is_same<T, T>(), "Type does not have a FromJS typemap");
    }
  }
  inline T *operator*() { return val_; }
};

} // namespace Typemap

} // namespace Nobind
