# nobind

Early experiments with a next-gen binding framework for Node.js / Node-API

Inspired by `pybind11` and `embind`, in turn inspired by the groundbreaking `Boost.Python`, this framework will use C++17 with fold expressions as baseline, allowing to generate separate precompiled wrappers for each method - instead of generic wrappers that process the arguments.

One day, it will be the first stepping stone towards a future **TWIG** - the *Templated Wrapper and Interface Generator*, a spiritual successor to SWIG that will:
  * Leverage modern C++ compilers and use C++ as its only language
  * Offer a single cross-platform and cross-language interface with Node.js, WASM and Python as initial targets
  * Support multi-threading and asynchronous execution
  * Use the same high-level approach as SWIG - by compiling the target library headers according to a set of rules - using the LLVM frontend

`nobind` will be usable on its own.

It is meant as an easy to use light-weight binding framework for simple projects. The first version won't support overloading and inheritance.
