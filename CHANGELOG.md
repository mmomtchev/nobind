# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0]

-   Support storing of a custom per-isolate data structure (`Napi::Env::GetInstanceData` and `Napi::Env::SetInstanceData`)
-   Specify the memory ownership rules for `Buffer`
-   Support returning references to nested objects
-   Fix [#16](https://github.com/mmomtchev/nobind/issues/16), global and `static` members can be `noexcept`
-   Fix a minor memory leak when calling a method with incorrect number of arguments

### [1.1.1] 2023-12-01

-   Fix [#7](https://github.com/mmomtchev/nobind/issues/7), escape the include directory path on Windows

## [1.1.0] 2023-11-30

-   Implement [#5](https://github.com/mmomtchev/nobind/issues/5), automatic object persistence in asynchronous mode
-   Fix [#3](https://github.com/mmomtchev/nobind/issues/3), compilation error when using `.ext()` class extension with arguments
-   Fix the module name in `binding.gyp` in the example

# [1.0.0] 2023-11-22

-   First release
