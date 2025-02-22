#pragma once

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

#include <napi.h>
// #undef NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER
