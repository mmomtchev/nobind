#pragma once

#ifdef NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER
#ifndef NODE_ADDON_API_REQUIRE_BASIC_FINALIZERS
#define NODE_ADDON_API_REQUIRE_BASIC_FINALIZERS
#endif
#endif

#ifndef NAPI_EXPERIMENTAL
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
