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
  * g++ 10.5 on Linux (the alternative choice on Ubuntu 20.04 and the default one on Ubuntu 22.04)
  * clang 13 on macOS (the default compiler on macOS 11)
  * MSVC 19.37 on Windows (Visual Studio 17.7 *aka* 2022)

It is meant as an easy to use entry-level light-weight binding framework for simple projects that target only Node.js.

Complex projects should continue to use SWIG which is cross-platform and cross-language.

**Currently, the project should be considered of a recent release quality.**

The first `npm` module to use it is [`@mmomtchev/ffmpeg`](https://github.com/mmomtchev/ffmpeg), you can check it for advanced usage examples.

A future compatible layer should allow to target both `embind` and `nobind17` with shared declarations.

Full `pybind11` compatibility is also a very long term goal - allowing a module to support both Node.js and Python.

You can use [`nobind-example-project`](https://github.com/mmomtchev/nobind-example-project) and [`hadron-nobind-example-project`](https://github.com/mmomtchev/hadron-nobind-example-project) as a template for creating a new `nobind17` based project using `node-gyp` or `hadron` as build system.

## Comparison vs SWIG Node-API

| Feature | SWIG Node-API | `nobind17` |
| --- | --- | --- |
| Design goal | Create bindings for (*almost*) any C or C++ calling semantic with (*almost*) native feel | Easy to use, easy to learn, be able to wrap most C++ calling semantics, including asynchronous methods, without understanding the Node.js memory management or thread model, be able to go a little further by tweaking the typemaps |
| Target use | Commercial-grade bindings for large C++ libraries | Very fast porting of C++ code with few methods/classes |
| Method of operation | Custom C++ header compiler, uses its own interface language, generates C++ code | Collection of C++ templates to be included in the project |
| Method of using | Must write metaprogramming code | Must enumerate the binded methods using C++ syntax |
| C++ requirements | C++11 | C++17 with some features such as wrapping of lambdas requiring C++20 |
| C++ types | Almost all, nested classes support is very limited | No functions pointers, no nested classes, `enum`s are not automatic |
| C++ preprocessing integration | Yes, can expose macros to JS | No |
| C++ namespaces | Can be exposed to JS with some limitations and manual work | Supported in C++ but not exposed to JS |
| C++ iterators | manual | automatic |
| `Buffer`s / `ArrayBuffer`s / `TypedArray`s | Yes | Only `Buffer`s for now |
| STL | Complete, supports both JS using C++ STLs without copying and C++ using JS types with copying | Limited, all passing of STL arguments is by copying |
| Async | Automatic | Automatic |
| Async locking | Yes, with automatic dead-lock prevention | Yes, but no deadlock prevention |
| Smart pointers | Yes | Not at the moment, but planned |
| TypeScript support | Automatic | Automatic |
| ES6 named exports for all C/C++ functions | Yes, automatic | No, must write it |
| WASM/Browser support | Yes | Not at the moment, but planned through `embind` compatibility |
| Cross-platform | Yes | Yes |
| Input language | Both C and C++ | Mostly C++, many usual C API semantics are not well supported |
| Target language | Most dynamic languages | An eventual abstraction layer between `nobind17`, `embind` and `pybind11` is planned in theory |
| Exposing C++ inheritance to JavaScript | Yes, automatic with implicit downcasting support, diamond inheritance is not supported | Yes, but no automatic downcasting support and no diamond inheritance |
| Overloading | Yes | Only for constructors, overloaded methods must be renamed to be usable in JS |
| Optional arguments with default values | Yes, automatic | No, all arguments become mandatory |
| Complex argument transformations (for example C++ expects (`char**, size_t*`) as input argument, JS expects `Buffer` as returned type) | Yes | Only `1`:`1` and `1`:`0` transformations of input arguments |
| Custom type casters | Yes | Yes |
| Interfacing between multiple modules | Yes | No |

## Usage

`nobind17` is a set of C++17 templates that must be included directly in the user project.

It is published as an npm package that will also install `node-addon-api`.

Starting from Node.js 18, C++17 is the default build mode for both Node.js itself and for addons. Unless you set manually `NAPI_VERSION` in your project, `nobind17@2.0` will default to `NAPI_VERSION=9` which requires Node.js 18.17, 20.3 or 21 and later. Older versions use `NAPI_VERSION-6` which allows backward compatibility of the generated binary addon with Node.js 14 and later - even when using Node.js 18 as the build platform.

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
// nobind will automatically include napi.h and it will
// define NAPI_VERSION and NAPI_EXPERIMENTAL
// If you include it yourself without these, you will be
// missing the synchronous finalizers
#include <nobind.h>

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
static constexpr auto myAttrs = Nobind::ReturnAsync |
                                Nobind::ReturnOwned |
                                Nobind::ReturnNullThrow;
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
  // Actually retrieving the value, this method can be
  // called from any thread, it should not interact with V8
  inline int Get() { return val_; }
  // Optional methods that, if present, will be called immediately
  // before or after returning from the call, can be used to implement
  // locking of the underlying C++ object if needed
  // Can be called in any thread, it should not interact with V8
  // For one call, Lock, Get and Unlock will always be called on the
  // same thread
  inline void Lock() {}
  inline void Unlock() {}

  // An optional public member may specify the number
  // of consumed JS arguments (considered 1 if not present)
  int Inputs;
  // Optionally, if the typemap has a state, specify only move
  // semantics, nobind17 can work with this type
  // Constructors and destructors will always be called only
  // on the main thread
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

#include <nobind.h>

int add(int a, int b) {
  return a + b;
}

NOBIND_MODULE(override_tmaps, m) {
  m.def<&add>("add");
}
```

Note that `nooverrides.h` must be included first, then the custom typemaps, then the rest of the templates with `nobind.h`.

A very good starting point for implementing a custom typemap are the standard number typemaps in [`nonumbermaps.h`](https://github.com/mmomtchev/nobind/blob/main/include/nonumbermaps.h), the string ones in [`nostringmaps.h`](https://github.com/mmomtchev/nobind/blob/main/include/nostringmaps.h) and the STL maps which are recursive in [`nostl.h`](https://github.com/mmomtchev/nobind/blob/main/include/nostl.h).

### Using `Buffer`s

Unless the C++ code has been designed for `nobind17`, using a `Buffer` will likely require creating custom wrappers to convert from and to `std::pair<uint8_t*, size_t>`:

```cpp
#include <fixtures/buffer.h>

// Nobind::Buffer is defined as follows:
// using Buffer = std::pair<uint8_t *, size_t>;

#include <nobind.h>

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

When JavaScript passes a `Buffer` to a C++ method, C++ receives a pointer to the underlying data region of the JS `Buffer` which is protected from the GC for duration of the call - including in async mode.

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

Eventually, as last resort, `Nobind::ReturnCopy` will copy the returned object. This might not be very efficient, but it will always be safe. The copy will be destroyed when the returned reference is GCed. `Nobind::ReturnCopy` works only for objects. Plain objects are always copied anyway, but it also allows to copy objects returned as references or pointers.

### Extending classes

Sometimes it is very handy to be able to add an additional class method in JavaScript that does not directly correspond to a C++ method. For example, the standard way of providing a method returning a readable string representation of an object is to overload the global `operator<<`. In JavaScript, the standard method is to replace the `Object.toString()`. This cannot be achieved with a simple helper function, because it will have to be a member of the binded class. In this case `nobind17` allows to define a special function of the form `RETTYPE Method(CLASS &, ARGS...)` and to register it as a class extension:

```cpp
std::string HelloToString(const Hello &);
```

```cpp
m.def<Hello>("Hello").ext<&ToString>("toString");
```

The first argument of the class extension must be `const CLASS &`, `CLASS &` or `Napi::Value` - it will contain the `this` object.

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
constexpr bool False = false;
NOBIND_MODULE(native, m) {
  m.Exports().Set("debug_build", Napi::Boolean::New(m.Env(), true));
  m.def<Hello>("Hello")
    .def<&False, Nobind::ReadOnly>(Napi::Symbol::WellKnown(m.Env(), "isConcatSpreadable"));
}
```

Automatically generating `Napi::Env` arguments without consuming input arguments is also possible, this allows to create a function without any arguments that returns a `Napi::Value`:

```cpp
Napi::Value get_string(Napi::Env env) {
  return Napi::String::New(env, "string w/o arguments");
}
```

All `Napi::Env` arguments will be automatically filled.

### Nested references

Consider the following C++ code:

```cpp
class Time {
  unsigned long timestamp;
public:
  Time(unsigned long v): timestamp(v) {};
};

class DateTime {
  Time time;
public:
  DateTime(Time v): time(v) {};
  Time &get() { return time; };
};
```

`DateTime` can returned a (non-`const`) reference to its member object `Time`. This reference should obviously use shared semantics as the newly created JS proxy object won't own the underlying C++ object. However, what will happen if the GC collects the parent object while JavaScript is still holding a reference to the returned nested object? This special case, which is somewhat common in the C++ world, requires special handling that can be enabled by using the `Nobind::ReturnNested` return attribute. In this case the returned reference will be bound the parent object which will be protected from the GC until the nested reference exists. This return attribute has a meaning only for class members and it is applied by default for class getters.

### Recursive typemaps

Typemaps can use recursion to reference other typemaps - this typemap for `std::map<std::string, T>` calls the existing typemaps for each contained object by using `FromJSValue<T>` and `ToJSValue<T>`.

```cpp
template <typename T> class FromJS<std::map<std::string, T>> {
  std::map<std::string, T> val_;

public:
  inline explicit FromJS(const Napi::Value &val) {
    if (!val.IsObject()) {
      throw Napi::TypeError::New(val.Env(), "Expected an object");
    }
    Napi::Object object = val.ToObject();
    for (auto prop : object) {
      val_.insert({prop.first.ToString().Utf8Value(), FromJSValue<T>(prop.second).Get()});
    }
  }

  inline M Get() { return val_; }
  FromJS(const FromJS &) = delete;
  FromJS(FromJS &&) = default;
};
```

### The Object Store

Starting from version 2, `nobind17` uses an object store. Each time a C++ object is wrapped, `nobind17` will remember this reference and as longer as the object is not garbage-collected, it will continue returning the same JS reference. This means that objects preserve their equality even if they cross multiple times the JS/C++ language barrier.

This is particularly important for `ReturnShared` objects and allows to avoid memory management issues related to having multiple JS wrappers for the same C++ object. With the Object Store, returning a C++ reference to an already existing object will reuse the existing JS reference. `ReturnCopy` will still copy the object and create a new underlying C++ object and a new JS wrapper for it.

The Object Store also permits to guarantee the locking mechanics since one C++ object can have only one JS wrapper proxy - not counting the special case of a separate JS wrappers for a base and a derived class which is not yet handled.

The Object Store can be disabled by defining the `NOBIND_NO_OBJECT_STORE` macro.

### Storing custom per-isolate data

Sometimes a module needs to store *"global"* data. With `node-addon-api` the proper way to store this data is in a per-isolate data structure - since Node.js is allowed to call the same instance from multiple independent isolates. To access the per-isolate storage with `nobind17`, declare the module specific structure and then use the standard `node-addon-api` calls to access it:

```cpp
struct PerIsolateData {
  Napi::ObjectReference exports;
};

NOBIND_MODULE_DATA(native, m, PerIsolateData) {
  m.Env().GetInstanceData<Nobind::EnvInstanceData<PerIsolateData>>()->exports =
      Napi::Persistent<Napi::Object>(m.Exports());
}
```

`nobind17` / `node-addon-api` will take care of creating and freeing this structure when new isolates are created and destroyed.

### C++ inheritance

Direct simple inheritance without virtual methods (*almost*) works out of the box. In order to properly set up the JavaScript `instanceof` operator, the class definitions must include the base class as a second template argument:

```cpp
m.def<Derived, Base>("Derived");
```

The only caveat is that this does not automatically inherit all the base class members. These must be declared separately for each class:

```cpp
m.def<Base>("Base")
  .cons<int>()
  .def<&Base::get>("get")
  .def<&Base::base_get>("base_get");
m.def<Derived, Base>("Derived")
  .cons<int>()
  .def<&Derived::get>("get")
  .def<&Derived::base_get>("base_get");
```

In this case, `get()` is a virtual method overriden in `Derived` and there is a single `base_get()` in `Base` that must also be explicitly declared in `Derived`. Resolution of virtual methods is left to the C++ compiler and follows the usual rules.

MSVC 2019, which is not fully C++17 compliant, requires a `static_cast` in this situation: see [here](https://github.com/mmomtchev/nobind17/blob/main/test/tests/inheritance.js). Later versions are fully compliant when using `/permissive-`.

When having to transpose multiple inheritance in C++ to JavaScript, it is possible to declare multiple implemented interfaces:

```cpp
m.def<Derived, Base, Interface1, Interface2>("Derived");
```

Currently this has an effect only on the TypeScript definitions which will include the corresponding `implements` declarations. `instanceof` in JavaScript will work only with the first base class.

### TypeScript support

Version 2 adds support for built-in automatically generated TypeScript definitions. These will be available inside the binary module in a special read-only variable called `__typescript`. This behaviour can be disabled if the module is built with the macro `NOBIND_NO_TYPESCRIPT_GENERATOR` defined. The default property name can be modified by defining `NOBIND_TYPESCRIPT_PROP`. In order to generate custom types, custom typemaps must have an additional method called `TSType()` returning an `std::string` with the TypeScript type:

```cpp
template <> class FromJS<bool> {
  bool val_;

public:
  inline explicit FromJS(const Napi::Value &val) {
    if (!val.IsBoolean()) {
      throw Napi::TypeError::New(val.Env(), "Expected a boolean");
    }
    val_ = val.ToBoolean().Value();
  }
  inline bool Get() { return val_; }

  static const std::string TSType() { return "boolean"; };
};
```

If the compiler supports RTTI and has the demangling ABI, defining `NOBIND_TYPESCRIPT_DEBUG` will produce type annotation comments with the original C++ types. 

#### Forward declarations

Generating TypeScript definitions may require the use of forward declarations - for example when two classes reference each other. In this case at least one of the classes must be declared before the other one is defined:

```cpp
m.decl<Base>("Base");
m.def<Dependant>("Dependant").cons<const Hello &>();
m.def<Base>("Base").cons<std::string &>();
```

The declaration and definition must use the same name. This allows the TypeScript generator to be able to correctly resolve `Base` objects when generating `Dependant`.

#### Dynamic class name

When creating generic typemaps, the current TypeScript name of the type can be obtained by calling `NoObjectWrap<T>::GetName()` - this requires that the class has at least been previously declared.

```cpp
static const std::string &TSType() { return NoObjectWrap<T>::::GetName(); };
```

#### Recursion

Recursive typemaps with TypeScript support can use the `FromTSType<T>` and `ToTSType<T, RETATTR = ReturnNullThrow>` typemaps to obtain the TypeScript definitions of the nested objects. Additionnaly, `createTSRecord<T, U>` and `createTSArray<T>` can be used to create `Record<>` and `[]` definitions.

#### Custom TypeScript fragments

Inserting a custom TypeScript code fragment anywhere at the root level in the code is possible with:

```cpp
m.typescript_fragment("export class CustomClass {}");
```

### Iterators

Iterators are mostly automatic but you must be aware that C++ iterators return references to the objects inside the container. The ownership of these objects is not always clear, but generally they are considered to be owned by the container.

`nobind17` offers two built-in interfaces to deal with iterable objects - one that copîes the returned objects to JavaScript and another one which returns shared references which prevent the container to be destroyed until the last returned object has been destroyed.

To define an iterator for the C++ class `Iterable`, instantiate the built-in `JSIterator` classe specifying either `ReturnCopy` or `ReturnNested` to define a JS-compatible iterator that has a `next` method. For TypeScript support it should implement `Nobind::TSIterator<Iterable>` - this will automatically define its TypeScript to return whatever type the C++ iterator returns - which will be `Iterable::iterator::value_type` as per the C++17 specifications:

```cpp
m.def<Nobind::JSIterator<Iterable, Nobind::ReturnCopy>, void, Nobind::TSIterator<Iterable>>(
      "_nobind_iterable_copy_iterator")
    .def<&Nobind::JSIterator<Iterable, Nobind::ReturnCopy>::next>("next");
```

Then in the definition of the `Iterable` class add the built-in helper `MakeJSIterator` as a `Symbol.iterator` extension method for the class:

```cpp
m.def<Iterable, void, Nobind::TSIterable<Iterable>>("Iterable")
    .ext<&Nobind::MakeJSIterator<Iterable1, Nobind::ReturnCopy>>(Napi::Symbol::WellKnown(m.Env(), "iterator"));
```

There is no `ReturnDefault` when working with iterators to further stress the fact that there is a decision to make.

Besides `ReturnNested` and `ReturnCopy` - which cover most cases, there are also cases where `ReturnShared` and `ReturnOwned` might be needed, but these are not completely safe and you should use them only if you understand the implications. `ReturnShared` means that the container does not own the returned object and it should never be freed - this is often the case for containers of pointers or containers of static objects. `ReturnOwned` is the most unusual - this means that the iterator is transfering the responsability of the returned object to its caller.

This expects that `Iterable` implements `std::input_iterator_tag` which is the most basic C++17 iterator - implementing only the pointer advancement operation and the indirection.

There is an example in [`iterator.cc`](https://github.com/mmomtchev/nobind/blob/main/test/tests/iterator.cc).

### Async locking

`nobind17@2` introduces automatic async locking. The built-in typemaps for object types will lock all the passed objects to any function for the duration of the call. This will automatically prevent reentrance of class objects. This however has three very important caveats:
  * If a the user code calls asynchronously a method which uses a C++ object - acquiring the lock on this object - any subsequent synchronous calls involving the same object will block the event loop until the first operations completes:
    ```typescript
    const data: Promise<DataType> = object.retrieveData(); // async
    // This will block the event loop until the first opertion completes:
    object.useData();
    ```
    This is impossible to avoid, as after launching the first operation, the interpreter will continue to synchronously execute the JS code and it will require to synchronously access the locked `object`. This however will have an identical behaviour without blocking the event loop:
    ```typescript
    const data: DataType = await object.retrieveData();
    object.useData();
    ```
    In this case the interpreter will yield the current context.

* If the user code launches two asynchronous operations involving the same object, they will run sequentially as expected. However, the second operation will sit waiting on the background thread pool which has a limited size. If the background pool has only 4 threads - the default Node.js value - launching 4 operations on the same object will lead to starvation of the thread pool. This is a good starting point for learning more: [Increase Node JS Performance With Libuv Thread Pool](https://dev.to/bleedingcode/increase-node-js-performance-with-libuv-thread-pool-5h10).

When implementing custom `FromJS` typemaps that provide locking, locking should be performed in the `Get()` method and unlocking in the `Release()` method. In case of an async operation, the actual locking and unlocking will happen in the background thread. When executing the operation, the main thread will only protect the object from being GCed, then once a background thread is available, the object will be actually locked to ensure that only a single thread is accessing it.

 * Automatic locking can lead to a deadlock. If there are two wrapped methods that can be called with multiple objects in a random order, there is a risk of a deadlock. For example when calling asynchronously `fn(a, b)` and `fn(b, a)` at almost the same time, the first one can lock `a` and wait for a lock on `b`, while the second one is holding `b` and waiting for a lock on `a`. The best way to ensure that this never happens is to always reference the objects in the same order.

Currently when using transformation of the STL classes - `std::vector` and `std::map` - and iterators - the locking is not recursive. This means that calling `some_method([a, b])` `a` and `b` won't be locked for the duration of the call. Similarly, objects that are nested references do not lock the parent object.

Async locking is another complex feature which certainly introduces new bugs and has a performance cost, it can be disabled by defining `NOBIND_NO_ASYNC_LOCKING`.

### R-value references

`nobind17` does not have built-in support for R-value references. These cannot really be expressed in JavaScript because a C++ method that expects an R-value reference will have to destroy the passed value in the parent scope - something that cannot be expressed in JavaScript.

Still, when dealing with a particular case which can be supported in JavaScript, it is possible to define custom typemaps to convert these arguments.

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

  *Custom typemaps must be included before `nobind.h` but after `nooverrides.h`.*

  *When overriding the builtin typemaps, you must use the special `Nobind::TypemapOverrides` namespace.*

  *Other typemaps must be in `Nobind::Typemap`.*

  *Depending on your types, you may need to also include pointer, reference or `const` typemaps - check the built-in implementation of `std::string` for an example.*

* Are you using MSVC?

  *MSVC has a number of problems with template argument deduction in its default compilation mode. The `/permissive-` and `/Zc` flags can help in some cases, or you can also use a `static_cast` to explicitly type your function pointer. `node-ffmpeg` includes a few cases of this type.*
  
  *Also, MSVC 2019 has a number of problems such as *C1001: Internal Compiler Error* on `static constexpr` local function variables used as non-type template arguments and some complex SFINAE constructs such as this one: [MSVC fails to specialize template with `std::enable_if` and a non-type argument](https://stackoverflow.com/questions/77698129/msvc-fails-to-specialize-template-with-stdenable-if-and-a-non-type-argument).*

* `assert(class_idx == 0 || class_idx == idx)` fails

  You most probably have multiple definitions for the same class.

## WASM compatbility

Although building to WASM using `emnapi` should be possible, this is considered out of scope for this project and you should be using `embind` which implements the same functionality directly in the `emscripten` compiler without adding additional layers (C++/`nobind` to `node-addon-api`, then `node-addon-api`/`emnapi` to `embind`).

## Developer info

Running single unit tests (in a debugger) is possible by doing:

```shell
cd test
node single configure <test>
node single build
node single run
```
