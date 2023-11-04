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

`nobind` is still not ready for use.

A future compatible layer should allow to target both `embind` and `nobind` with shared declarations.

Full `pybind11` compatibility is also a very long term goal - allowing a module to support both Node.js and Python.
