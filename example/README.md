# Examples

There are four examples here:

* `hello-world`

  A most basic example with a simple C++ class exposed to JS.

And three examples for handling an argument with an object with named fields on the JS side that transforms into a `struct` on the C++ side:

* `handle-struct`
  
  Which copies objects when passing from JS to C++.

* `handle-struct-with-smart-ptr`
  
  Which uses `nobind17` smart pointers support to get actual pointers with automatic reference handling.

* `handle-struct-with-plain-ptr`

  Which gives a raw pointer with some caveats, avoid this unless really necessary.

You can refer to the unit tests for more examples, such as handling overloading, inheritance, arrays, maps, buffers and iterators.
