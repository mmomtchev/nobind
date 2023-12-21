# nobind17

Experimental next-gen binding framework for Node.js / Node-API inspired by `pybind11`

Inspired by `pybind11` and `embind`, in turn inspired by the groundbreaking `Boost.Python`.

This framework is designed around C++17 fold expressions.

It has one defining characteristic that sets it apart from `pybind11` and `embind` - every wrapper is statically generated at compile time and has no run-time state. All the state information is `constexpr` and it is encoded in the template parameters. The wrappers are instantiated by obtaining a pointer to the wrapper function.

This allows for both a (slightly) better performance and code simplicity.

The unit tests run on:
  * g++ 9.4 on Linux (the default compiler on Ubuntu 20.04)
  * clang 13 on macOS (the default compiler on macOS 11)
  * MSVC 19.29 on Windows (Visual Studio 16.11 *aka* 2019)

However because of edge cases when it comes to C++17 support, the recommended compiler versions are:
  * g++ 10.5 on Linux (the alternative choice on Ubuntu 20.04)
  * clang 13 on macOS (the default compiler on macOS 11)
  * MSVC 19.37 on Windows (Visual Studio 17.7 *aka* 2022) on Windows

It is meant as an easy to use entry-level light-weight binding framework for simple projects.

Complex projects should continue to use SWIG which is cross-platform and cross-language.

**Currently, the project should be considered of a new release quality.**

The first `npm` module to use it is [`@mmomtchev/ffmpeg`](https://github.com/mmomtchev/ffmpeg), you can check it for advanced usage examples.

A future compatible layer should allow to target both `embind` and `nobind17` with shared declarations.

Full `pybind11` compatibility is also a very long term goal - allowing a module to support both Node.js and Python.

## Comparison vs SWIG Node-API

| Feature | SWIG Node-API | `nobind17` |
| --- | --- | --- |
| Design goal | Create bindings for (*almost*) any C++ code with (*almost*) native feel | Easy to use, easy to learn |
| Target use | Commercial-grade bindings for large C++ libraries | Very fast porting of C++ code with few methods/classes |
| Method of operation | Custom C++ header compiler, uses its own interface language, generates C++ code | Collection of C++ templates to be included in the project |
| Method of using | Must write metaprogramming code | Must enumerate the binded methods using C++ syntax |
| C++ requirements | C++11 | C++17 with some features such as wrapping of lambdas requiring C++20 |
| C++ types | No function pointers | No `enum` and functions pointers |
| C++ preprocessing integration | Yes, can expose macros to JS | No |
| `Buffer`s / `ArrayBuffer`s / `TypedArray`s | Yes | Only `Buffer`s for now |
| STL | Limited, but will evolve; Supports direct use of C++ STL containers from JavaScript without copying | Limited, all passing of STL arguments is by copying |
| Async | Automatic | Automatic |
| Async locking | Yes with automatic dead-lock prevention | Not for 1.0 |
| Smart pointers | Yes | Possible but not for 1.0 |
| TypeScript support | Automatic | No, must write the typings |
| WASM/Browser support | Yes | Not for 1.0, but planned through `embind` compatibility |
| Cross-platform | Yes | Yes |
| Cross-language | Yes, most dynamic languages | An eventual abstraction layer between `nobind17`, `embind` and `pybind` is planned in theory |
| Exposing C++ inheritance to JavaScript | Yes, automatic with implicit downcasting support | Yes, but no downcasting support and `instanceof` requires a small kludge in the JavaScript wrapper (see [here](https://github.com/mmomtchev/nobind17/blob/main/test/tests/inheritance.js)) |
| Overloading | Yes | Only for constructors, overloaded methods must be renamed to be usable in JS |
| Optional arguments | Yes, automatic | Yes, manual
| Complex argument transformations (for example C++ expects (`char**, size_t*`) as input argument, JS expects `Buffer` as returned type) | Yes | Only `n`:`1` transformations of input arguments |
| Custom type casters | Yes | Yes |
| Interfacing between multiple modules | Yes | No |

## Usage

`nobind17` is a set of C++17 templates that must be included directly in the user project.

It is published as an npm package that will also install `node-addon-api`.

Starting from Node.js 18, C++17 is the default build mode for both Node.js itself and for addons. Unless you set manually `NAPI_VERSION` in your project, `nobind17` will default to `NAPI_VERSION=6` which will allow backward compatibility of the generated binary addon with Node.js 14 and later - even when using Node.js 18 as the build platform.

`nobind17` is designed to be very easy to use - there is no learning curve at all - while allowing to deal with the most common situations that arise when creating bindings for C++ libraries to be used from Node.js.

The following tutorial should be enough to get you started with your C++ project.

You can also check [node-ffmpeg](https://github.com/mmomtchev/node-ffmpeg) as an example for a large project using `nobind17`.

### The environment

Create a a `binding.gyp`, then create a `package.json` for your project and install `nobind17`:

`binding.gyp`
```python
{
  'target_defaults': {
    'includes': [
      # These are the correct compiler options
      # to enable C++ exceptions with node-gyp
      'except.gypi'
    ]
  },
  'targets': [
    {
      'target_name': 'my-shiny-cpp-bindings',
      'sources': [
        # This is the file that contains your bindings
        # (from the tutorial below)
        'src/my-shiny-cpp-bindings.cc'
        # List your C++ files here
        # If you have a large library, check
        # https://github.com/mmomtchev/node-ffmpeg
        # for inspiration, it builds ffmpeg with conan
      ],
      'include_dirs': [
        '<!@(node -p "require(\'node-addon-api\').include")',
        '<!@(node -p "require(\'nobind17\').include")'
      ]
    }
  ]
}
```

```shell
npm init # ... answer questions
npm install nobind17
cp node_modules/node-addon-api/except.gypi .
```

You will be building your project with `node-gyp configure build`. `node-gyp` is usually installed globally.

C++17 is the default build mode starting from Node.js 18.x. If you 

### Module definition

Let's try to wrap a simple C++ class:

```cpp
class Hello {
public:
  std::string name;
  Hello(const std::string &s) : name(s) {}
  std::string Greet(const std::string &s) {
    std::stringstream r;
    r << "hello " << s << " " << name_;
    return r.str();
  }
};
```

Start by creating a module:

```cpp
#include <nobind17.h>

// Define a new module
NOBIND_MODULE(my_cpp_bindings, m) {
  // Expose a C++ class called Hello
  m.def<Hello>("Hello")
    // Include a constructor with a single const std::string & argument
    .cons<const std::string &>();
}
```

### Adding methods

`nobind17` supports global methods and instance and static class methods. All of them are declared by using `.def()`:

```cpp
// Expose a global function global_fn
m.def<&global_fn>("global_fn");
m.def<MyClass>("Hello")
  .cons<std::string &>()
  // Expose a class method (whether it is static or instance)
  .def(&Hello::Greet, "greet");
```

`nobind17` will identify the type of the class method, static methods will be available through the class itself and instance methods will be available through the object instance.

A class can have multiple constructors, including a default one (use `<>` for its arguments). The number of arguments on the JavaScript side determine which one will be used. If there a multiple constructors expecting the same number of arguments, they will be tried in the order of their declaration - the first one which is able to convert its arguments will win.

Overloaded methods, other than constructors, must be explicitly resolved and each signature must have a different name in JavaScript.

Arguments will be automatically converted. The C++ type of the wrapped function selects the type converter. The basic types supported out of the box are:

| JavaScript type | C++ type |
| --- | --- |
| `number`  | `int`, `short`, `long`, `unsigned`, `unsigned short`, `unsigned long`, `long long`, `unsigned long long`, `double`, `float` |
| `string` | `std::string`, `char *` |
| `boolean` | `bool` |
| `object` | `std::map<string, T>` (*all properties must have the same type*) |
| `Array` | `std::vector<T>` (*all items must have the same type*) |
| instances of class registered to `nobind17` | class object, pointers and references |
| `Buffer` | `std::pair<uint8_t *, size_t>` |
| A raw V8 `Napi::Value` | `Napi::Value` |

Additional custom type converters can be registered by the user.

### Getters and setters

Global as well as class static and instance variables can be exposed with the same type conversion rules:

```cpp
// Expose a read-only global variable version
m.def<&version, Nobind::ReadOnly>("version");
m.def<MyClass>("Hello")
  .cons<std::string &>()
  // Expose a class instance variable with getter and setter
  .def(&Hello::name, "name");
```

`nobind17` will automatically determine if the object is a static or an instance one.

### Creating wrappers and using STLs

Using STLs usually requires creating a wrapper function unless the original C++ function has been designed from the ground up to work with `nobind17`:

```cpp
// A function that receives a JS array of Hello objects
// It calls the .Greet() method of each object
// and returns a JS array of strings
std::vector<std::string>
GreetAll(const std::string &title, const std::vector<Hello *> &array) {
  std::vector<std::string> r;
  r.reserve(array.size());
  for (auto obj : array) {
    r.push_back(obj->Greet(title));
  }
  return r;
}

NOBIND_MODULE(array, m) {
  m.def<&GreetAll>("greetAll");
  m.def<MyClass>("Hello")
    // Include a constructor with a single std::string & argument
    .cons<std::string &>()
    .def<&Hello::Greet>("greet");
}
```

Used from JavaScript this function will have the following semantics:

```js
const output = dll.greetAll('Mr', [
  new dll.Hello('Brown'),
  new dll.Hello('Orange'),
  new dll.Hello('Pink')
]);
typeof output[0] === 'string'
```

`std::vector` can be of any supported type - including known registered object types, pointers or references to them, primitives types or any other additional custom type. `nobind17` will take care to transform the pointers and the references to JS objects.

### C++ exceptions

Methods that raise a C++ exception will result in a normal JavaScript exception in the JavaScript code.

Building with C++ exceptions enabled is mandatory.

### Async methods

Methods can be made to run in a background thread from the `libuv` thread pool and to return a `Promise` to be resolved with the returned value:

```js
m.def<Hello>("Hello")
  .def<&Hello::Greet, Nobind::ReturnAsync>("greetAsync");
```

Everything is fully automatic. Raising a C++ exception will reject the `Promise`.

Enabling async mode will allow the JS user to potentially call the C++ method while a previous invocation is still running. If the C++ method is not fully reentrant, a wrapper with a lock mechanism should be implemented.

### `nullptr`

By default, when a C++ method returns a `nullptr`, `nobind17` will convert it to `null` in JavaScript. This behavior can be overridden by specifying `Nobind::ReturnNullThrow` as a return attribute - in this case the method will throw. If the method is asynchronous, it will reject.

### Combining attributes

Attributes can be combined with `operator|`, however if compiling in C++17 mode (the default settings for `node-gyp`), only `static constexpr` variables can be used as non-type template arguments:

```cpp
static constexpr auto myAttrs = Nobind::ReturnAsync | Nobind::ReturnOwned | Nobind::ReturnNullThrow;
```

In later standards this requirement has been relaxed. Also, MSVC 2019 chokes on `static constexpr` local function variables used as non-type template arguments with an *C1001: Internal Compiler Error* - use global variables if you have to support it.

### Custom type converters

Custom type converters can be declared as follows:

```cpp
// This example overrides the default `int` typemaps
// with typemaps that expect and return strings

// Start by including this file
#include <nooverrides.h>

namespace Nobind {
// Typemaps that will be overriding built-ins must live
// in this namespace to override
// (typemaps for new types must be in Nobind::Typemap)
namespace TypemapOverrides {

// They consist of two simple classes templated on the C++ type
// (the C++ type is the determning type)
// This one will be called whenever nobind17 needs to convert
// a JS argument to C++ int
template <> class FromJS<int> {
  int val_;

public:
  // The first part will be called from the V8 context
  // It must import the value and store it so that it can
  // be accessed without V8
  // It must check if the JS argument is of the correct type
  inline explicit FromJS(Napi::Value val): Inputs(1) {
    if (!val.IsString()) {
      throw Napi::TypeError::New(val.Env(), "Expected a string");
    }
    val_ = std::atoi(val.ToString().Utf8Value().c_str());
  }
  // The second part may be called from a background thread
  // It should Expected access V8
  inline int Get() { return val_; }
  // An optional public member may specify the number
  // of consumed JS arguments (considered 1 if not present)
  int Inputs;
  // Optionally, if the typemap has a costly state, only move
  // semantics may be specified, nobind17 can work with this type
  FromJS(const FromJS &) = delete;
  FromJS(FromJS &&) = default;
};

// This typemap will be used when C++ returns an int
// It must create a value for JS
template <> class ToJS<int> {
  Napi::Env env_;
  int val_;

public:
  // The first part may be called from a background thread
  // It should simply store the value for later use
  inline explicit ToJS(Napi::Env env, int val) : env_(env), val_(val) {}
  // The second part will be called on the main V8 thread
  // It should produce a JS value
  inline Napi::Value Get() { return Napi::String::New(env_, std::to_string(val_)); }
  // Optionally, if the typemap has a costly state, only move
  // semantics may be specified, nobind17 can work with this type
  ToJS(const ToJS &) = delete;
  ToJS(ToJS &&) = default;
};
} // namespace TypemapOverrides

} // namespace Nobind

#include <nobind17.h>

int add(int a, int b) {
  return a + b;
}

NOBIND_MODULE(override_tmaps, m) {
  m.def<&add>("add");
}
```

### Using `Buffer`s

Unless the C++ code has been designed for `nobind17`, using a `Buffer` will likely require creating custom wrappers to convert from and to `std::pair<uint8_t*, size_t>`:

```cpp
#include <fixtures/buffer.h>

// Nobind::Buffer is defined as follows:
// using Buffer = std::pair<uint8_t *, size_t>;

#include <nobind17.h>

// These are the underlying C++ functions that use buffers
// We want to call them from JS
void get_buffer(uint8_t *&, size_t &);
void put_buffer(uint8_t *, size_t);

// These wrappers are what makes them nobind17-compatible
Nobind::Typemap::Buffer nobind_get_buffer() {
  Nobind::Typemap::Buffer buf;
  get_buffer(buf.first, buf.second);
  return buf;
}
void nobind_put_buffer(Nobind::Typemap::Buffer buf) {
  put_buffer(buf.first, buf.second);
}

NOBIND_MODULE(buffer, m) {
  m.def<&nobind_get_buffer>("get_buffer")
  .def<&nobind_put_buffer>("put_buffer");
}
```

When C++ returns a `Buffer` object, that buffer is considered owned and it will be freed upon the destruction of the Node.js `Buffer` object by the garbage-collector.

When JavaScript passes a `Buffer` to a C++ method, data is copied - which is the safest but not the most efficient way to transfer it. Sharing the memory between C++ and JavaScript is possible in many cases - but must be carefully implemented in a custom typemap - especially when using async.

### Returning objects and factory functions

Before continuing with this section, we should explain the notion of a JS proxy.

Each C++ object is created with `new` and destroyed with `delete` in the C++ heap. These objects are not directly visible from JavaScript. What is visible from JavaScript is called a JS proxy - a pure JS object that contains a hidden pointer to the underlying C++ object. This JS object is managed by the V8 GC.

This means that functions that return C++ objects need to be compatible with the GC rules in JavaScript. For every function, other than a constructor, that returns an object, there must be clear rules on who frees the C++ object.

By default, `nobind17` will consider that it owns objects returned as pointers and that it does not own objects returned as references. This behavior can be modified with an attribute:

```cpp
class Chained {
public:
  Chained();
  Chained *Factory();
  Chained &Do();
};

NOBIND_MODULE(chained, m) {
  m.def<Chained>("Chained")
    .cons<>()
    // Nobind::ReturnOwned is the default behavior for pointers
    .def<&Chained::Factory, Nobind::ReturnOwned>("create");
    // Nobind::ReturnShared is the default behavior for references
    .def<&Chained::Do, Nobind::ReturnShared>("do");
}
```

`.do()` is a method that can be chained:
```js
const o = new Chained;
o.do().do().do();
```

The `Nobind::ReturnShared` signals `nobind17` that C++ objects returned by this method should not be considered new objects and should not be freed when the JS proxy is collected by the GC.

`.create()` is a method that creates new objects. The `Nobind::ReturnOwned` signals `nobind17` that C++ objects returned by this method should be considered new objects and should be freed when the GC destroys the JS proxy.

Also, be sure to check [#1](https://github.com/mmomtchev/nobind17/issues/1) for a very important warning about shared references.

### Extending classes

Sometimes it is very handy to be able to add an additional class method in JavaScript that does not directly correspond to a C++ method. For example, the standard way of providing a method returning a readable string representation of an object is to overload the global `operator<<`. In JavaScript, the standard method is to replace the `Object.toString()`. This cannot be achieved with a simple helper function, because it will have to be a member of the binded class. In this case `nobind17` allows to define a special function of the form `RETTYPE Method(CLASS &, ARGS...)` and to register it as a class extension:

```cpp
std::string HelloToString(const Hello &);
```

```cpp
m.def<Hello>("Hello").ext<&ToString>("toString");
```

Currently, there is no way to register a getter with a function in order to override the `[@@toStringTag]` property.

### Directly accessing the underlying `node-addon-api`

C++ functions that expect `Napi::Value` arguments or return `Napi::Value` results will skip the type conversions. This can be used to interact directly with the underlying Node.js API.

Unlike raw Node-API, C++ functions will receive their `Napi::Value`s with the usual C++ convention:

```cpp
Napi::Value add(Napi::Value a, Napi::Value b);
```

Mixing is also supported:
```cpp
int add(Napi::Value a, int b);
```

In this case only the first argument will contain the raw V8 value.

It is also possible to access the `exports` and `env` objects when initializing the module:
```cpp
NOBIND_MODULE(native, m) {
  static constexpr bool False = false;

  m.Exports().Set("debug_build", Napi::Boolean::New(m.Env(), true));
  m.def<Hello>("Hello")
    .def<&False, Nobind::ReadOnly>(Napi::Symbol::WellKnown(m.Env(), "isConcatSpreadable"));
}
```

### Troubleshooting

Most of the work that `nobind17` does happens during the C++ compilation of the project. It is at that moment that the templates will be instantiated.

As it is often the case with C++ compilation, the errors may be hard to read.

When encountering compilation errors, start with this quick checklist:

* Does the error message mention missing typemaps such as `FromJS`/`ToJS`?

  *You are trying to expose types that `nobind17` does not know how to convert, you need a custom typemap.*

* Is the method that does not compile an overloaded method?

  *You need to use `static_cast` to manually resolve the overloading.*

* Is the method that does not compile inherited from a base class?

  *You need to use the base class name.*

* Is the custom typemap not being picked up?

  *Custom typemaps must be included before `nobind17.h` but after `nooverrides.h`.*

  *When overriding the builtin typemaps, you must use the special `Nobind::TypemapOverrides` namespace.*

  *Other typemaps must be in `Nobind::Typemap`.*

  *Depending on your types, you may need to also include pointer, reference or `const` typemaps - check the built-in implementation of `std::string` for an example.*

* Are you using MSVC?

  *MSVC has a number of problems with template argument deduction in its default compilation mode. The `/permissive-` and `/Zc` flags can help in some cases, or you can also use a `static_cast` to explicitly type your function pointer. `node-ffmpeg` includes a few cases of this type.*
  
  *Also, MSVC 2019 has a number of problems such as *C1001: Internal Compiler Error* on `static constexpr` local function variables used as non-type template arguments and some complex SFINAE constructs such as this one: [MSVC fails to specialize template with `std::enable_if` and a non-type argument](https://stackoverflow.com/questions/77698129/msvc-fails-to-specialize-template-with-stdenable-if-and-a-non-type-argument).*

## Developer info

Running single unit tests (in a debugger) is possible by doing:

```shell
cd test
node single configure <test>
node single build
node single run
```
