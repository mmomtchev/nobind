#include <nodebug.h>
#include <nonapi.h>

#include <mutex>
#include <unordered_map>
#include <vector>

namespace Nobind {

// An object store keeps weak references to the already constructed JS wrappers
// for C++ objects. When a new object needs to be wrapped, it can be looked up
// by the C++ pointer value. When the JS wrapper is GCed, the ObjectStore
// entry is invalidated. The element is deleted either when someone checks for
// it, or in the NoObjectWrap destructor.
//
// There are three benefits of the ObjectStore:
// * it avoids recreating wrappers for existing objects (ie works as cache
//   improving performance)
// * is ensures that only a single wrapper exists for a single object,
//   avoiding memory management problems when one wrapper owns the object,
//   while another one has a ReturnShared type
// * it allows the user to check for object equality even when crossing
//   multiple times the C++/JS interface
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
//
// Every type has its own object store, because two objects can share the
// same pointer in some cases:
//   struct A { int a; };
//   struct B { A obj; } b;
// Pointers to b and b.obj will have the same value, but will be
// treated differently depending on the type.
//
// Another case is a function which returns an object as being
// of its base type and another one, which returns the same object but
// as its derived type. We don't want to set the type in stone after
// the first time the object has been returned.
//
// The second case, can create another potentially disastrous situation:
// * A derived object has a second wrapper as its base class
// * The derived object and its derived wrapper are GCed
// * The second wrapper remains alive with a dangling pointer
//   This object is a problem on its own
// * Another object is created with the same pointer
//   Now that wrapper points to a different object
// * The new object and the new wrapper create a second
//   object store item where there are now two identical
//   pointers leading to inconsistent behaviour
//
// The object store problem is solved in the Put() method, but
// currently there is no good solution for the dangling pointer.

template <typename T> class ObjectStore {
  // Is this really needed?
  std::mutex lock;
  std::vector<std::unordered_map<T, Napi::Reference<Napi::Value>>> object_store;

  template <typename U> Napi::Value GetLocked(size_t class_idx, U *ptr) {
    NOBIND_VERBOSE_TYPE(STORE, U, ptr, "Get from object store: ");
    auto &store = object_store.at(class_idx);
    auto el = store.find(static_cast<T>(ptr));
    if (el == store.end()) {
      NOBIND_VERBOSE(STORE, "not there\n");
      return Napi::Value{};
    }

    Napi::Value js = el->second.Value();
    if (js.IsEmpty()) {
      NOBIND_VERBOSE(STORE, "expired\n");
      // The chain is still here but the goat is nowhere to be found
      store.erase(static_cast<T>(ptr));
      return Napi::Value{};
    }

    NOBIND_VERBOSE(STORE, "found\n");
    return js;
  }

public:
  template <typename U> Napi::Value Get(size_t class_idx, U *ptr) {
    std::lock_guard guard{lock};
    return GetLocked(class_idx, ptr);
  }

  template <typename U> NOBIND_INLINE void Put(size_t class_idx, U *ptr, Napi::Value js) {
    std::lock_guard guard{lock};

    NOBIND_VERBOSE_TYPE(STORE, U, ptr, "create in object store\n");
    auto &store = object_store.at(class_idx);

    // insert or assign to replace existing elements, refer to the
    // last part of the comment at the top
    store.insert_or_assign(static_cast<T>(ptr), Napi::Reference<Napi::Value>::New(js));
  }

  template <typename U> NOBIND_INLINE void Expire(size_t class_idx, U *ptr, Napi::Value js) {
    std::lock_guard guard{lock};

    Napi::Value stored = GetLocked(class_idx, ptr);
    NOBIND_VERBOSE_TYPE(STORE, U, ptr, "Expire from object store: ");
    if (stored.IsEmpty()) {
      NOBIND_VERBOSE(STORE, "already expired\n");
      return;
    }
    if (js.IsEmpty()) {
      // If the stored reference is not empty and this one is empty
      // (which can happen only without basic finalizers), the only
      // explanation is that the stored object is a new wrapper
      NOBIND_VERBOSE(STORE, "already expired (no basic finalizers)");
      return;
    }
    // Are we expiring the right object?
    if (stored == js) {
      NOBIND_VERBOSE(STORE, "expiring\n");
      auto &store = object_store.at(class_idx);
      store.erase(static_cast<T>(ptr));
    } else {
      NOBIND_VERBOSE(STORE, "new object present\n");
    }
  }

  // We don't care for const, no two objects of the same type can have the same pointer anyway
  template <typename U> NOBIND_INLINE Napi::Value Get(size_t idx, const U *ptr) {
    return Get(idx, const_cast<U *>(ptr));
  }
  template <typename U> NOBIND_INLINE void Put(size_t idx, const U *ptr, Napi::Value js) {
    return Put(idx, const_cast<U *>(ptr), js);
  }
  template <typename U> NOBIND_INLINE void Expire(size_t idx, const U *ptr, Napi::Value js) {
    Expire(idx, const_cast<U *>(ptr), js);
  }

  void Init(size_t s) {
    for (size_t i = 0; i < s; i++)
      object_store.emplace_back();
  }

  void Flush() {
    NOBIND_VERBOSE(STORE, "flushing object store\n");
    for (auto &store : object_store)
      store.clear();
  }
};
} // namespace Nobind
