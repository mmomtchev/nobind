#pragma once
#include <functional>
#include <napi.h>
#include <tuple>
#include <type_traits>

#include <notypes.h>

namespace Nobind {

// The JS proxy object type
template <typename CLASS> class NoObjectWrap : public Napi::ObjectWrap<NoObjectWrap<CLASS>> {
public:
  NoObjectWrap(const Napi::CallbackInfo &);
  static Napi::Function GetClass(Napi::Env, const char *,
                                 const std::vector<Napi::ClassPropertyDescriptor<NoObjectWrap<CLASS>>> &);

  // Constructor wrapper
  template <typename... ARGS> void ConsWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    CheckArgLength<ARGS...>(env, info.Length());
    if constexpr (sizeof...(ARGS) > 0) {
      std::tuple<std::remove_const_t<std::decay_t<ARGS>>...> args;

      std::apply(
          [&info](auto &...args) {
            size_t i = 0;
            ((args = *Nobind::FromJS<std::remove_reference_t<decltype(args)>>(info[i++])), ...);
          },
          args);

      self = new CLASS(std::make_from_tuple<CLASS>(args));
    } else {
      self = new CLASS;
    }
  }

  // The first function of the member method wrapper trio (same std::integral_constant trick)
  template <auto CLASS::*FUNC> Napi::Value MethodWrapper(const Napi::CallbackInfo &info) {
    return MethodWrapper(info, std::integral_constant<decltype(FUNC), FUNC>{});
  }

  template <typename T, T CLASS::*MEMBER> Napi::Value GetterWrapper(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return *Typemap::ToJS<T>(env, self->*MEMBER);
  }

  template <typename T, T CLASS::*MEMBER> T Getter() { return self->*MEMBER; }

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
    if constexpr (sizeof...(ARGS) > 0) {
      RETURN result = (self->*FUNC)(*Nobind::FromJS<ARGS>(info[I])...);
      return *Typemap::ToJS<RETURN>(env, result);
    } else {
      RETURN result = (self->*FUNC)();
      return *Typemap::ToJS<RETURN>(env, result);
    }
  }

  CLASS *self;
  bool owned;
};

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
  // TODO: Avoid copying
  owned = true;
  const std::string cons_name = "__cons_"s + std::to_string(info.Length());
  auto cons = info.This().ToObject().Get("__proto__").ToObject().Get(cons_name.c_str());
  if (cons.IsFunction()) {
    std::vector<napi_value> args(info.Length());
    for (size_t i = 0; i < info.Length(); i++)
      args[i] = info[i];
    cons.As<Napi::Function>().Call(info.This(), args);
    return;
  }
  throw Napi::TypeError::New(info.Env(), "No constructor with " + std::to_string(info.Length()) + " arguments found");
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
  std::vector<void (NoObjectWrap<CLASS>::*)(const Napi::CallbackInfo &info)> constructors;

public:
  template <auto CLASS::*MEMBER> ClassDefinition &def(const char *name) {
    if constexpr (std::is_member_function_pointer<decltype(MEMBER)>()) {
      typename NoObjectWrap<CLASS>::InstanceMethodCallback wrapper =
          &NoObjectWrap<CLASS>::template MethodWrapper<MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::InstanceMethod(name, wrapper));
    } else {
      typename NoObjectWrap<CLASS>::InstanceGetterCallback getter =
          &NoObjectWrap<CLASS>::template GetterWrapper<decltype(getMemberPointerType(MEMBER)), MEMBER>;
      properties.emplace_back(NoObjectWrap<CLASS>::template InstanceAccessor(name, getter, nullptr));
    }
    return *this;
  }

  template <typename... ARGS> ClassDefinition &cons() {
    void (NoObjectWrap<CLASS>::*wrapper)(const Napi::CallbackInfo &info) =
        &NoObjectWrap<CLASS>::template ConsWrapper<ARGS...>;
    // TODO: fix the leak (InstanceMethod trivially copies a const char *)
    std::string *name = new std::string("__cons_"s + std::to_string(sizeof...(ARGS)));
    properties.push_back(NoObjectWrap<CLASS>::InstanceMethod(name->c_str(), wrapper));
    return *this;
  }

  ClassDefinition(const char *name, Napi::Env env, Napi::Object exports)
      : name_(name), env_(env), exports_(exports), properties(), constructors() {}

  ~ClassDefinition() { exports_.Set(name_, NoObjectWrap<CLASS>::GetClass(env_, name_, properties)); }
};

} // namespace Nobind
