#include <nonapi.h>

#include <map>

namespace Nobind {

// An object store keeps weak references to the already constructed JS wrappers
// for C++ objects. When a new object needs to be wrapped, it can be searched
// by the C++ pointer value. When the JS wrapper is GCed, the ObjectStore
// entry is invalidated. The element is deleted either when someone checks for
// it, or in the NoObjectWrap destructor.
//
// There are three benefits of the ObjectStore:
// * it avoids recreating wrappers for existing objects (ie performance)
// * is ensures that only a single wrapper exists for a single object,
//   avoiding memory management problems when one wrapper owns the object,
//   while another one has a ReturnShared type
// * it allows the user to check for object equality
//
// Most of the complexity here is to avoid this particular scenario:
// * A C++ object is wrapped and placed in the object store
// * The GC collects the JS wrapper and invalidates the weak reference,
//   it schedules the calling of the NoObjectWrap destructor, the
//   weak reference is now isEmpty()
// * The same C++ object is wrapped again, creating a new entry for the same
//   C++ pointer
// * The NoObjectWrap destructor of the first wrapper is finally invoked and
//   it deletes the second wrapper

template <typename T> class ObjectStore {
  std::map<T, Napi::Reference<Napi::Value>> object_store;

public:
  template <typename U> bool Has(U *ptr) { return object_store.count(ptr) > 0; }
  template <typename U> Napi::Value Get(U *ptr) {
    if (object_store.count(static_cast<T>(ptr)) == 0)
      return Napi::Value{};

    Napi::Reference<Napi::Value> &ref = object_store.at(static_cast<T>(ptr));
    if (ref.IsEmpty()) {
      // The chain is still here but the goat is nowhere to be found
      object_store.erase(static_cast<T>(ptr));
      return Napi::Value{};
    }
    Napi::Value js = ref.Value();

    return js;
  }

  template <typename U> inline void Put(U *ptr, Napi::Value js) {
    object_store.emplace(static_cast<T>(ptr), Napi::Reference<Napi::Value>::New(js));
  }

  template <typename U> inline void Expire(U *ptr, Napi::Value js) {
    Napi::Value stored = Get(ptr);
    if (stored.IsEmpty())
      return;
    // Are we expiring the right object?
    if (stored == js) {
      object_store.erase(static_cast<T>(ptr));
    }
  }

  // We don't care for const, no two objects can have the same pointer anyway
  template <typename U> inline Napi::Value Get(const U *ptr) { return Get(const_cast<U *>(ptr)); }
  template <typename U> inline void Put(const U *ptr, Napi::Value js) { return Put(const_cast<U *>(ptr), js); }
  template <typename U> inline void Expire(const U *ptr, Napi::Value js) { Expire(const_cast<U *>(ptr), js); }
};
} // namespace Nobind
