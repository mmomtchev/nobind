#pragma once
#define NOBIND_OBJECT_STORE_DEBUG

#if !defined(NAPI_EXPERIMENTAL) && !defined(NOBIND_NO_BASIC_FINALIZERS)
#define NAPI_EXPERIMENTAL
#endif

#ifndef NAPI_VERSION
#define NAPI_VERSION 6
#elif NAPI_VERSION < 6
#error NAPI_VERSION 6 is the minimum supported target (Node.js >=14)
#endif

#if __cplusplus < 201703L
#error C++17 is required to build with nobind17
#endif

#if defined(NOBIND_TYPESCRIPT_DEBUG) || defined(NOBIND_OBJECT_STORE_DEBUG)
#include <cxxabi.h>
#define TYPENAME(T) demangle<std::string>(typeid(T).name())

// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
// (templated to be able to include in a header file)
template <typename T> T demangle(const char *name) {

  int status = -4;
  std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  return (status == 0) ? res.get() : name;
}
#endif

#include <napi.h>
