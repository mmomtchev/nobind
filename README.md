# nobind

Experimental next-gen binding framework for Node.js / Node-API

Inspired by `pybind11` and `embind`, in turn inspired by the groundbreaking `Boost.Python`, this framework uses C++17 with fold expressions as baseline, allowing to generate separate precompiled wrappers for each method - instead of generic wrappers that process the arguments.

It is tested with
  * g++ 11.4 on Linux
  * clang 13 on macOS
  * MSVC 2022 on Windows

It is meant as an easy to use entry-level light-weight binding framework for simple projects.

Complex projects should continue to use SWIG which is cross-platform and cross-language.

The first version won't support inheritance and will lack many of the advanced features of `pybind11`.

`nobind` is still not finished, but it may be usable.

A future compatible layer should allow to target both `embind` and `nobind` with shared declarations.

Full `pybind11` compatibility is also a very long term goal - allowing a module to support both Node.js and Python.

## Comparison vs SWIG Node-API

| Feature | SWIG Node-API | `nobind` |
| --- | --- | --- |
| Design goal | Create native-feeling bindings for (almost) any C++ library | Easy to use, easy to learn |
| Target use | Commercial-grade bindings for large C++ libraries | Very fast porting of C++ code with few methods/classes |
| Method of operation | Custom C++ header compiler, uses its own interface language, generates C++ code | Collection of C++ templates to be included in the project |
| Method of using | Must write metaprogramming code | Must enumerate the binded methods |
| C++ requirements | C++11 | C++17 with some features such as wrapping of lambdas requiring C++20 |
| All primitives C++ types | everything | everything except `enum` |
| C++ preprocessing integration | yes, can expose macros to JS | no |
| STL | limited, but will evolve | limited |
| Async | out-of-the-box | planned to be out-of-the-box for 1.0 |
| Smart pointers | yes | possible but not for 1.0 |
| TypeScript support | automatic | no, must write the typings |
| WASM/Browser support | yes | not for 1.0, but planned through `embind` compatibility |
| Cross-platform | yes | yes |
| Cross-language | yes, most dynamic languages | an eventual abstraction layer between `nobind`, `embind` and `pybind` is planned in theory |
| C++ inheritance | yes | no |
| Overloading | yes | only for constructors, overloaded methods must be renamed to be usable in JS |
| Complex argument transformations (for example C++ expects (`char**, size_t*`) as input argument, JS expects `Buffer` as returned type) | yes | must manually write a wrapper |
| Custom type casters | yes | planned for 1.0 |
| Interfacing between multiple modules | yes | no |
