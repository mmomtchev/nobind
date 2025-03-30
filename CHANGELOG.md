# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# [2.0.0]

-   TypeScript support
-   Object store (fixes [#1](https://github.com/mmomtchev/nobind/issues/1))
-   Automatic async locking
-   Implement synchronous garbage-collection through basic finalizers - this pushes the minimum supported `node-addon-api` to 8.3.0 and it requires Node.js 18.20, 20.12, 21.6 or 22+ - check for `NODE_API_EXPERIMENTAL_HAS_POST_FINALIZER` macro to ensure that the feature is enabled
-   Initial interaction with the garbage collector statistics - currently only for objects that do not grow once created (objects are reported to the GC with their initial size)
-   Automatic support for C++ iterators via built-in helpers
-   Basic smart pointers support
-   Support for returning non-copy constructible objects from C++ by C++17 copy elision
-   Shortcuts for defining both a sync and an async version methods in one call
-   New `Nobind::ReturnCopy` return attribute allows to always copy the returned object
-   Fix [#30](https://github.com/mmomtchev/nobind/issues/30), `m.ext<T>(...)` accepts only functions with `T &` as first parameter
-   Fix [#34](https://github.com/mmomtchev/nobind/issues/34), getters of members of class type return dangling references

### [1.2.1] 2024-07-16

-   Fix [#21](https://github.com/mmomtchev/nobind/issues/21), the default typemaps do not accept C++ POD objects

## [1.2.0] 2024-01-05

-   Support storing of a custom per-isolate data structure (`Napi::Env::GetInstanceData` and `Napi::Env::SetInstanceData`)
-   Specify the memory ownership rules for `Buffer`
-   Support returning references to nested objects
-   Fix [#16](https://github.com/mmomtchev/nobind/issues/16), global and `static` members can be `noexcept`
-   Fix a minor memory leak when calling a method with incorrect number of arguments
-   More meaningful exceptions for constructors to aid debugging

### [1.1.1] 2023-12-01

-   Fix [#7](https://github.com/mmomtchev/nobind/issues/7), escape the include directory path on Windows

## [1.1.0] 2023-11-30

-   Implement [#5](https://github.com/mmomtchev/nobind/issues/5), automatic object persistence in asynchronous mode
-   Fix [#3](https://github.com/mmomtchev/nobind/issues/3), compilation error when using `.ext()` class extension with arguments
-   Fix the module name in `binding.gyp` in the example

# [1.0.0] 2023-11-22

-   First release
