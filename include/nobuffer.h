#pragma once
#include <cstddef>

#include <notypes.h>

namespace Nobind {

namespace Typemap {

// These typemaps transfer Buffers by copying the underlying data
// It is also possible to do it without copying if managing the GC

// In C++ a Node::Buffer decomposes to std::pair<uint8_t *, size_t>
using Buffer = std::pair<uint8_t *, size_t>;

template <> class FromJS<Buffer> {
  Buffer val_;

public:
  inline explicit FromJS(Napi::Value val) {
    if (!val.IsBuffer()) {
      throw Napi::TypeError::New(val.Env(), "Not a Buffer");
    }
    Napi::Buffer buf = val.As<Napi::Buffer<uint8_t>>();
    val_ = {new uint8_t[buf.ByteLength()], buf.ByteLength()};
    memcpy(val_.first, buf.Data() + buf.ByteOffset(), buf.ByteLength());
  }

  inline Buffer operator*() { return val_; }
};

template <const ReturnAttribute &RETATTR> class ToJS<Buffer, RETATTR> {
  Napi::Env env_;
  Buffer val_;

public:
  inline explicit ToJS(Napi::Env env, Buffer val) : env_(env), val_(val) {}
  inline Napi::Value operator*() { return Napi::Buffer<uint8_t>::Copy(env_, val_.first, val_.second); }
};

} // namespace Typemap

} // namespace Nobind
