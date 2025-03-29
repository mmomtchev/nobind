#pragma once
#include <notypes.h>

#include <memory>

namespace Nobind {

namespace Typemap {

// JS object -> C++ std::shared_ptr
// This typemap creates a new std::shared_ptr pointing to the JS
// object with a custom deleter that holds a persistent reference
// C++ can keep a copy of this shared_ptr, the deleter
// will signal the JS GC that C++ is done with this object
template <typename T> class FromJS<std::shared_ptr<T>> {
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;
  std::shared_ptr<T> val_;
  OBJCLASS *wrapper_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "shared_ptr FromJS works only with objects");
    OBJCLASS::CheckInstance(val);
    wrapper_ = OBJCLASS::Unwrap(val.ToObject());
    T *underlying = wrapper_->Get();
    Napi::ObjectReference *persistent = new Napi::ObjectReference;
    val_ = std::shared_ptr<T>(underlying, [persistent](void *p) {
      NOBIND_VERBOSE_TYPE(OBJECT, T, static_cast<T *>(p), "Finalizing shared_ptr passed to C++\n");
      // Deleting the last copy of the shared_ptr
      // will release the persistent
      delete persistent;
    });
    Napi::Persistent(val.ToObject());
  }
  NOBIND_INLINE std::shared_ptr<T> Get() { return val_; }

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

// C++ std::shared_ptr to JS object
// This typemap wraps the object as a normal pointer
// and registers a custom finalizer that deletes
// the owning shared_ptr upon GC collection
template <typename T, const ReturnAttribute &RETATTR> class ToJS<std::shared_ptr<T>, RETATTR> {
  Napi::Env env_;
  std::shared_ptr<T> val_;
  using OBJCLASS = NoObjectWrap<std::remove_cv_t<std::remove_reference_t<T>>>;

public:
  NOBIND_INLINE explicit ToJS(Napi::Env env, std::shared_ptr<T> &val) : env_(env), val_(val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "shared_ptr ToJS works only with objects");
  }

  NOBIND_INLINE Napi::Value Get() {
    if constexpr (RETATTR.isCopy()) {
      // No need for shenanigans when we copy
      return OBJCLASS::template New<true>(env_, new T{*val_});
    } else {
      // Keep a copy of the shared_ptr in the custom finalizer
      // and wrap this as a normal pointer
      Napi::Value r = OBJCLASS::template New<RETATTR.ShouldOwn<true>()>(env_, val_.get());
      OBJCLASS *wrapper = OBJCLASS::Unwrap(r.ToObject());
      std::shared_ptr<T> *owner = new std::shared_ptr<T>(val_);
      wrapper->SetFinalizer([owner](Napi::BasicEnv env, T *p) -> void {
        NOBIND_VERBOSE_TYPE(OBJECT, T, p, "Finalizing shared_ptr obtained from C++\n");
        delete owner;
      });
      return r;
    }
  }

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

} // namespace Typemap
} // namespace Nobind
