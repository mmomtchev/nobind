#pragma once
#include <nohelpers.h>
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
  using TYPE = std::remove_cv_t<std::remove_reference_t<T>>;
  using OBJCLASS = NoObjectWrap<TYPE>;
  std::shared_ptr<TYPE> val_;
  OBJCLASS *wrapper_;

public:
  NOBIND_INLINE explicit FromJS(const Napi::Value &val) {
    static_assert(std::is_object_v<T> && !std::is_scalar_v<T>, "shared_ptr FromJS works only with objects");
    Napi::Env env = val.Env();
    OBJCLASS::CheckInstance(val);
    wrapper_ = OBJCLASS::Unwrap(val.ToObject());
    TYPE *underlying = wrapper_->Get();
    Napi::ObjectReference *persistent = new Napi::ObjectReference;
    *persistent = Napi::Persistent(val.ToObject());
    val_ = std::shared_ptr<TYPE>(underlying, [persistent, env](void *p) {
      NOBIND_VERBOSE_TYPE(OBJECT, T, static_cast<T *>(p), "Finalizing shared_ptr passed to C++\n");
      // Deleting the last copy of the shared_ptr
      // will release the persistent
      RunOnJSMainThread<BaseEnvInstanceData>(env, [persistent, p]() {
        NOBIND_VERBOSE_TYPE(OBJECT, T, static_cast<T *>(p), "Releasing JS reference\n");
        delete persistent;
      });
    });
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
      if constexpr (RETATTR.isShared()) {
        // ReturnShared does not have a finalizer, this is a normal pointer
        return OBJCLASS::template New<RETATTR.ShouldOwn<true>()>(env_, val_.get());
      } else {
        // Keep a copy of the shared_ptr in the custom finalizer
        // and wrap this as a normal pointer
        // (the destruction of the object will destroy the lambda and shared_ptr)
        return OBJCLASS::template New<RETATTR.ShouldOwn<true>()>(
            env_, val_.get(), [owner = this->val_](Napi::BasicEnv env, T *p) -> void {
              NOBIND_VERBOSE_TYPE(OBJECT, T, p, "Finalizing shared_ptr obtained from C++\n");
            });
      }
    }
  }

  static const std::string &TSType() { return OBJCLASS::GetName(); };
};

} // namespace Typemap
} // namespace Nobind
