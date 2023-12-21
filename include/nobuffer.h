#pragma once
#include <cstddef>

#include <notypes.h>

namespace Nobind {

namespace Typemap {

// These typemaps transfer Buffers by copying the underlying data
// It is also possible to do it without copying if properly managing the GC
// but this requires very through understanding of all the underlying
// mechanisms by the user of nobind17

// In C++ a Node::Buffer decomposes to std::pair<uint8_t *, size_t>
using Buffer = std::pair<uint8_t *, size_t>;

// When calling C++ with a JS Buffer, a copy of the Buffer
// is created for the duration of the call
// This means that the typemap is completely safe for use with async
// code but it is not the most efficient way to transfer data
template <> class FromJS<Buffer> {
  Buffer val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if (!val.IsBuffer()) {
      throw Napi::TypeError::New(val.Env(), "Expected a Buffer");
    }
    Napi::Buffer buf = val.As<Napi::Buffer<uint8_t>>();
    val_ = {new uint8_t[buf.ByteLength()], buf.ByteLength()};
    memcpy(val_.first, buf.Data() + buf.ByteOffset(), buf.ByteLength());
  }

  inline Buffer Get() { return val_; }

  inline ~FromJS() {
    delete[] val_.first;
    val_.first = nullptr;
    val_.second = 0;
  }

  FromJS(const FromJS &) = delete;

  // When moving, we must be sure that the old copy
  // won't get freed
  inline FromJS(FromJS &&rhs) {
    val_ = rhs.val_;
    rhs.val_.first = nullptr;
    rhs.val_.second = 0;
  };
};

// When receiving a Buffer from C++ we consider that
// ownership has been transferred to us
template <const ReturnAttribute &RETATTR> class ToJS<Buffer, RETATTR> {
  Napi::Env env_;
  Buffer val_;

public:
  inline explicit ToJS(Napi::Env env, Buffer val) : env_(env), val_(val) {}
  inline Napi::Value Get() {
#ifdef NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
    // Node-API does not support external buffers (Electron)
    Napi::Buffer buffer = Napi::Buffer<uint8_t>::Copy(env_, val_.first, val_.second);
    delete[] val->data;
    return buffer;
#else
    // Node-API supports external buffers (Node.js)
    return Napi::Buffer<uint8_t>::New(env_, val_.first, val_.second, [](Napi::Env, uint8_t *data) { delete[] data; });
#endif
  }

  ToJS(const ToJS &) = delete;
  ToJS(ToJS &&) = delete;
};

} // namespace Typemap

} // namespace Nobind
