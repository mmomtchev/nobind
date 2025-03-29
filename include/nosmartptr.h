#pragma once
#include <notypes.h>

#include <memory>

namespace Nobind {

struct NobindNullDeleter {
  void operator()(void const *) const {}
};

namespace Typemap {

// JS object -> C++ std::shared_ptr
// This typemap creates a new std::shared_ptr pointing to the JS
// object with a null deleter
// TODO: Link this with the JS GC so that C++ can keep this pointer
template <typename T> class FromJS<std::shared_ptr<T>> {
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
  T *val_;
  OBJCLASS *wrapper_;
  Napi::ObjectReference persistent_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "shared_ptr FromJS works only with objects");
    OBJCLASS::CheckInstance(val);
    wrapper_ = OBJCLASS::Unwrap(val.ToObject());
    val_ = wrapper_->Get();
    persistent_ = Napi::Persistent(val.ToObject());
  }
  NOBIND_INLINE std::shared_ptr<T> Get() { return std::shared_ptr<T>(val_, NobindNullDeleter{}); }

#ifndef NOBIND_NO_ASYNC_LOCKING
  NOBIND_INLINE void Lock() NOBIND_NOEXCEPT {
    if (wrapper_)
      wrapper_->Lock();
  }
  NOBIND_INLINE void Unlock() NOBIND_NOEXCEPT {
    if (wrapper_)
      wrapper_->Unlock();
  }
#endif

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

} // namespace Typemap
} // namespace Nobind
