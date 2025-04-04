#pragma once
#ifndef NOBIND_TYPESCRIPT_PROP
#define NOBIND_TYPESCRIPT_PROP "__typescript"
#endif

#include <noattributes.h>
#include <nonapi.h>
#include <ostream>
#include <string>
#include <vector>

#if defined(NOBIND_TYPESCRIPT_DEBUG)
#define TSTYPE_DEBUG(S, T) ((S).empty() ? "" : "/* C++ type: "s + TYPENAME(T) + " */ "s + (S))
#else
#define TSTYPE_DEBUG(S, T) S
#endif

namespace Nobind {

namespace Typemap {} // namespace Typemap

template <typename T> class TSIterable {};
template <typename T> class TSIterator {};

using namespace std::string_literals;

// Helpers to determine if the typemap contains a TSType
template <typename T> class JSTypemapHasTSType {
  template <typename U> static constexpr decltype(std::declval<U &>().TSType, bool()) test(int) { return true; }
  template <typename U> static constexpr NOBIND_INLINE bool test(...) { return false; }

public:
  static constexpr bool value = test<T>(int());
};

template <typename CLASS> class NoObjectWrap;

// Resolve a C++ argument type to TS argument type
template <typename T> std::string FromTSType() {
  if constexpr (std::is_constructible_v<TypemapOverrides::FromJS<std::remove_cv_t<T>>, const Napi::Value &>) {
    if constexpr (JSTypemapHasTSType<TypemapOverrides::FromJS<std::remove_cv_t<T>>>::value) {
      return TSTYPE_DEBUG(TypemapOverrides::FromJS<std::remove_cv_t<T>>::TSType(), T);
    } else {
      return TSTYPE_DEBUG("unknown"s, T);
    }
  } else {
    if constexpr (JSTypemapHasTSType<Typemap::FromJS<std::remove_cv_t<T>>>::value) {
      return TSTYPE_DEBUG(Typemap::FromJS<std::remove_cv_t<T>>::TSType(), T);
    } else {
      return TSTYPE_DEBUG("unknown"s, T);
    }
  }
}

// Resolve a C++ return type to a TS return type
template <typename T, const ReturnAttribute &RETATTR> std::string ToTSType() {
  if constexpr (std::is_void_v<T>) {
    return TSTYPE_DEBUG("void"s, T);
  } else if constexpr (std::is_constructible_v<TypemapOverrides::ToJS<std::remove_cv_t<T>>, const Napi::Env &, T>) {
    if constexpr (JSTypemapHasTSType<TypemapOverrides::ToJS<std::remove_cv_t<T>>>::value) {
      if constexpr (RETATTR.isReturnNullAccept()) {
        return TSTYPE_DEBUG(TypemapOverrides::ToJS<std::remove_cv_t<T>>::TSType() + " | null", T);
      } else {
        return TSTYPE_DEBUG(TypemapOverrides::ToJS<std::remove_cv_t<T>>::TSType(), T);
      }
    } else {
      return TSTYPE_DEBUG("unknown"s, T);
    }
  } else {
    if constexpr (JSTypemapHasTSType<Typemap::ToJS<std::remove_cv_t<T>>>::value) {
      if constexpr (RETATTR.isReturnNullAccept()) {
        return TSTYPE_DEBUG(Typemap::ToJS<std::remove_cv_t<T>>::TSType() + " | null", T);
      } else {
        return TSTYPE_DEBUG(Typemap::ToJS<std::remove_cv_t<T>>::TSType(), T);
      }
    } else {
      return TSTYPE_DEBUG("unknown"s, T);
    }
  }
}

// Construct a string with all function argument
template <typename... ARGS> NOBIND_INLINE std::string FromTSTypes() {
  std::vector<std::string> types{FromTSType<ARGS>()...};
  std::string types_text;
  for (size_t i = 0; i < types.size(); i++) {
    if (types[i].empty())
      continue;
    if (!types_text.empty())
      types_text += ", "s;
    types_text += "arg"s + std::to_string(i) + ": "s + types[i];
  }
  return types_text;
}

// Construct a string with all implements arguments
template <typename... INTERFACES> NOBIND_INLINE std::string FromTSTInterfaces() {
  std::vector<std::string> types{FromTSType<INTERFACES>()...};
  std::string types_text;
  for (size_t i = 0; i < types.size(); i++) {
    if (!types_text.empty())
      types_text += ", "s;
    else
      types_text += " implements ";
    types_text += types[i];
  }
  return types_text;
}

// FunctionSignature is a three-stage function (refer to the comments in nofunction.h)
// It constructs TypeScript signatures for global function ands static class members
// Third stage
template <const ReturnAttribute &RETATTR, auto *FUNC, typename RETURN, typename... ARGS, std::size_t... I,
          typename NAME = const char *>
NOBIND_INLINE std::string FunctionSignature(NAME name, const char *prefix, std::index_sequence<I...>) {
  std::string types_text = FromTSTypes<ARGS...>();
  std::string return_text;
  std::string resolved_name;
  if constexpr (std::is_same_v<Napi::Symbol, NAME>) {
    resolved_name = "["s + ((Napi::Symbol)name).ToObject().Get("description").ToString().Utf8Value() + "]"s;
  } else {
    resolved_name = std::string{name};
  }
  if constexpr (RETATTR.isAsync())
    return_text = "Promise<"s + ToTSType<RETURN, RETATTR>() + ">"s;
  else
    return_text = ToTSType<RETURN, RETATTR>();
  return std::string{prefix} + resolved_name + "("s + types_text + "): "s + return_text + ";\n"s;
}

// Second stage, two variants (except and noexcept)
template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...),
          typename NAME = const char *>
NOBIND_INLINE std::string FunctionSignature(NAME name, const char *prefix,
                                            std::integral_constant<RETURN (*)(ARGS...), FUNC>) {
  return FunctionSignature<RETATTR, FUNC, RETURN, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(ARGS...) noexcept,
          typename NAME = const char *>
NOBIND_INLINE std::string FunctionSignature(NAME name, const char *prefix,
                                            std::integral_constant<RETURN (*)(ARGS...) noexcept, FUNC>) {
  return FunctionSignature<RETATTR, FUNC, RETURN, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}

// First stage
template <const ReturnAttribute &RETATTR, auto *FUNC, typename NAME = const char *>
std::string FunctionSignature(NAME name, const char *prefix) {
  return FunctionSignature<RETATTR>(name, prefix, std::integral_constant<decltype(FUNC), FUNC>{});
}

// Class extension, second stages, call the FunctionSignature 3rd stage
// Variant 1, This() is CLASS & or const CLASS &
template <const ReturnAttribute &RETATTR, typename RETURN, typename CLASS, typename... ARGS,
          RETURN (*FUNC)(CLASS &, ARGS...), typename NAME = const char *>
NOBIND_INLINE std::string ExtensionSignature(NAME name, const char *prefix,
                                             std::integral_constant<RETURN (*)(CLASS &, ARGS...), FUNC>) {
  return FunctionSignature<RETATTR, FUNC, RETURN, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
// Variant 2, This() is Napi::Value
template <const ReturnAttribute &RETATTR, typename RETURN, typename... ARGS, RETURN (*FUNC)(Napi::Value, ARGS...),
          typename NAME = const char *>
NOBIND_INLINE std::string ExtensionSignature(NAME name, const char *prefix,
                                             std::integral_constant<RETURN (*)(Napi::Value, ARGS...), FUNC>) {
  return FunctionSignature<RETATTR, FUNC, RETURN, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
// Class extension, first stage
template <const ReturnAttribute &RETATTR, auto *FUNC, typename NAME = const char *>
std::string ExtensionSignature(NAME name, const char *prefix) {
  return ExtensionSignature<RETATTR>(name, prefix, std::integral_constant<decltype(FUNC), FUNC>{});
}

// Construct a TypeScript signature for a class constructor
template <typename... ARGS> std::string ConstructorSignature() {
  std::string types_text = FromTSTypes<ARGS...>();
  return "constructor("s + types_text + ");\n"s;
}

// Member function, 3 stages
// Third stage
template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, auto FUNC, typename... ARGS, std::size_t... I,
          typename NAME = const char *>
NOBIND_INLINE std::string MethodSignature(NAME name, const char *prefix, std::index_sequence<I...>) {
  std::string types_text = FromTSTypes<ARGS...>();
  std::string return_text;
  std::string resolved_name;
  if constexpr (std::is_same_v<Napi::Symbol, NAME>) {
    resolved_name = "["s + ((Napi::Symbol)name).ToObject().Get("description").ToString().Utf8Value() + "]"s;
  } else {
    resolved_name = std::string{name};
  }
  if constexpr (RETATTR.isAsync())
    return_text = "Promise<"s + ToTSType<RETURN, RETATTR>() + ">"s;
  else
    return_text = ToTSType<RETURN, RETATTR>();
  return std::string{prefix} + resolved_name + "("s + types_text + "): "s + return_text + ";\n"s;
}

// Second stage, 4 variants:
// - regular, const, noexcept and const noexcept
template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
          RETURN (BASE::*FUNC)(ARGS...), typename NAME = const char *>
NOBIND_INLINE std::string MethodSignature(NAME name, const char *prefix,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...), FUNC>) {
  return MethodSignature<RETATTR, BASE, RETURN, FUNC, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
          RETURN (BASE::*FUNC)(ARGS...) const, typename NAME = const char *>
NOBIND_INLINE std::string MethodSignature(NAME name, const char *prefix,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) const, FUNC>) {
  return MethodSignature<RETATTR, BASE, RETURN, FUNC, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
          RETURN (BASE::*FUNC)(ARGS...) noexcept, typename NAME = const char *>
NOBIND_INLINE std::string MethodSignature(NAME name, const char *prefix,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) noexcept, FUNC>) {
  return MethodSignature<RETATTR, BASE, RETURN, FUNC, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}
template <const ReturnAttribute &RETATTR, typename BASE, typename RETURN, typename... ARGS,
          RETURN (BASE::*FUNC)(ARGS...) const noexcept, typename NAME = const char *>
NOBIND_INLINE std::string MethodSignature(NAME name, const char *prefix,
                                          std::integral_constant<RETURN (BASE::*)(ARGS...) const noexcept, FUNC>) {
  return MethodSignature<RETATTR, BASE, RETURN, FUNC, ARGS...>(name, prefix, std::index_sequence_for<ARGS...>{});
}

// First stage
template <const ReturnAttribute &RET, auto FUNC, typename NAME = const char *>
std::string MethodSignature(NAME name, const char *prefix) {
  return MethodSignature<RET>(name, prefix, std::integral_constant<decltype(FUNC), FUNC>{});
}

// Class property
template <const PropertyAttribute &PROP, typename TYPE, typename NAME = const char *>
std::string PropertySignature(NAME name, const char *prefix) {
  std::string resolved_name;
  if constexpr (std::is_same_v<Napi::Symbol, NAME>) {
    resolved_name = "["s + ((Napi::Symbol)name).ToObject().Get("description").ToString().Utf8Value() + "]"s;
  } else {
    resolved_name = std::string{name};
  }
  return std::string{prefix} + (PROP.isReadOnly() ? "readonly "s : ""s) + resolved_name + ": "s + ToTSType<TYPE>() +
         ";\n";
}

// Global variable
template <const PropertyAttribute &PROP, typename TYPE, typename NAME = const char *>
std::string GlobalSignature(NAME name, const char *prefix) {
  std::string resolved_name;
  if constexpr (std::is_same_v<Napi::Symbol, NAME>) {
    resolved_name = "["s + ((Napi::Symbol)name).ToObject().Get("description").ToString().Utf8Value() + "]"s;
  } else {
    resolved_name = std::string{name};
  }
  return std::string{prefix} + (PROP.isReadOnly() ? "const "s : "var "s) + resolved_name + ": "s + ToTSType<TYPE>() +
         ";\n";
}

template <typename T, typename U> std::string createTSRecord() {
  return "Record<"s + FromTSType<T>() + ", "s + FromTSType<U>() + ">"s;
}

template <typename T> std::string createTSArray() { return FromTSType<T>() + "[]"s; }

namespace Typemap {
template <typename T> class FromJS<TSIterable<T>> {
public:
  static std::string TSType() { return "Iterable<"s + FromTSType<typename T::iterator::value_type>() + ">"s; }
};

template <typename T> class FromJS<TSIterator<T>> {
public:
  static std::string TSType() { return "Iterator<"s + FromTSType<typename T::iterator::value_type>() + ">"s; }
};

// IteratorResult follows the Napi::Value conversion rules
template <typename T> class ToJS<JSIteratorResult<T>> : public ToJS<Napi::Value> {
  Napi::Value val_;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, JSIteratorResult<T> val) : ToJS<Napi::Value>(env, val) {}
  static std::string TSType() { return "IteratorResult<"s + FromTSType<T>() + ">"s; }
};

} // namespace Typemap
} // namespace Nobind
