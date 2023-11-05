#pragma once
#include <cstddef>

#include <notypes.h>

namespace Nobind {

namespace Typemap {

// These typemaps transfer Buffers _without_ copying the underlying data
// (unless NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED is defined)

// In C++ a Node::Buffer decomposes to std::pair<uint8_t *, size_t>
using Buffer = std::pair<uint8_t *, size_t>;

template <> class FromJS<Buffer> {
  Buffer val_;

public:
  inline FromJS(Napi::Value val) {
    if (!val.IsBuffer()) {
      throw Napi::TypeError::New(val.Env(), "Not a Buffer");
    }
    Napi::Buffer buf = val.As<Napi::Buffer<uint8_t>>();
    val_ = {buf.Data() + buf.ByteOffset(), buf.ByteLength()};
  }

  inline Buffer operator*() { return val_; }
};

template <> class ToJS<Buffer> {
  Napi::Env env_;
  Buffer val_;

public:
  inline ToJS(Napi::Env env, Buffer val) : env_(env), val_(val) {}
  inline Napi::Value operator*() { return Napi::Buffer<uint8_t>::NewOrCopy(env_, val_.first, val_.second); }
};

} // namespace Typemap

} // namespace Nobind
