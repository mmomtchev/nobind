#pragma once
#include <cstddef>

#include <notypes.h>

namespace Nobind {

namespace Typemap {

// In C++ a Node::Buffer decomposes to std::pair<uint8_t *, size_t>
using Buffer = std::pair<uint8_t *, size_t>;

const std::string Buffer_tstype = "Buffer"s;

// When calling C++ with a JS Buffer, C++ receives a pointer
// to the Node.js Buffer region
// The JS Buffer object is protected from the GC for the
// duration of the call
template <> class FromJS<Buffer> {
  Buffer val_;
  Napi::ObjectReference persistent;

public:
  NOBIND_INLINE explicit FromJS(Napi::Value val) {
    if (!val.IsBuffer()) {
      throw Napi::TypeError::New(val.Env(), "Expected a Buffer");
    }
    Napi::Buffer buf = val.As<Napi::Buffer<uint8_t>>();
    val_ = {buf.Data(), buf.ByteLength()};
    persistent = Napi::Persistent(val.As<Napi::Object>());
  }

  NOBIND_INLINE Buffer Get() { return val_; }

  FromJS(const FromJS &) = delete;
  NOBIND_INLINE FromJS(FromJS &&) = default;

  static const std::string &TSType() { return Buffer_tstype; }
};

// When receiving a Buffer from C++ we consider that
// ownership has been transferred to us
template <const ReturnAttribute &RETATTR> class ToJS<Buffer, RETATTR> {
  Napi::Env env_;
  Buffer val_;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, Buffer val) : env_(env), val_(val) {}
  NOBIND_INLINE Napi::Value Get() {
#ifdef NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
    // Node-API does not support external buffers (Electron)
    // C++ receives a copy with the original being freed immediately
    Napi::Buffer buffer = Napi::Buffer<uint8_t>::Copy(env_, val_.first, val_.second);
    delete[] val_.first;
    return buffer;
#else
    if constexpr (RETATTR.isCopy()) {
      Napi::Buffer buffer = Napi::Buffer<uint8_t>::Copy(env_, val_.first, val_.second);
      delete[] val_.first;
      return buffer;
    }
    // C++ receives ownership of the buffer which is freed upon collection of the JS Buffer object by the GC
    return Napi::Buffer<uint8_t>::New(env_, val_.first, val_.second,
                                      [](Napi::BasicEnv, uint8_t *data) { delete[] data; });
#endif
  }

  ToJS(const ToJS &) = delete;
  ToJS(ToJS &&) = delete;

  static const std::string &TSType() { return Buffer_tstype; }
};

} // namespace Typemap

} // namespace Nobind
