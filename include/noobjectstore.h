#include <nonapi.h>

#include <map>
#include <mutex>
#include <vector>

#ifdef NOBIND_OBJECT_STORE_DEBUG
#define NOBIND_OBJECT_STORE_TYPE(T) printf("[%s] ", TYPENAME(T).c_str());
#define NOBIND_OBJECT_STORE_VERBOSE(...) printf(__VA_ARGS__);
#else
#define NOBIND_OBJECT_STORE_TYPE(T)
#define NOBIND_OBJECT_STORE_VERBOSE(...)
#endif

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
// treated differently depending on the type

template <typename T> class ObjectStore {
  std::mutex lock;
  std::vector<std::map<T, Napi::Reference<Napi::Value>>> object_store;

public:
  template <typename U> Napi::Value Get(size_t class_idx, U *ptr) {
    std::lock_guard guard{lock};

    NOBIND_OBJECT_STORE_TYPE(U);
    NOBIND_OBJECT_STORE_VERBOSE("Get %p: ", ptr);
    if (object_store.at(class_idx).count(static_cast<T>(ptr)) == 0) {
      NOBIND_OBJECT_STORE_VERBOSE("not there\n");
      return Napi::Value{};
    }

    Napi::Reference<Napi::Value> &ref = object_store.at(class_idx).at(static_cast<T>(ptr));
    if (ref.IsEmpty()) {
      NOBIND_OBJECT_STORE_VERBOSE("expired\n");
      // The chain is still here but the goat is nowhere to be found
      object_store.at(class_idx).erase(static_cast<T>(ptr));
      return Napi::Value{};
    }
    Napi::Value js = ref.Value();

    NOBIND_OBJECT_STORE_VERBOSE("found\n");
    return js;
  }

  template <typename U> inline void Put(size_t class_idx, U *ptr, Napi::Value js) {
    std::lock_guard guard{lock};

    NOBIND_OBJECT_STORE_TYPE(U);
    NOBIND_OBJECT_STORE_VERBOSE("Create %p\n", ptr);
    object_store.at(class_idx).emplace(static_cast<T>(ptr), Napi::Reference<Napi::Value>::New(js));
  }

  template <typename U> inline void Expire(size_t class_idx, U *ptr, Napi::Value js) {
    std::lock_guard guard{lock};

    NOBIND_OBJECT_STORE_TYPE(U);
    NOBIND_OBJECT_STORE_VERBOSE("Expire %p: ", ptr);
    Napi::Value stored = Get(class_idx, ptr);
    if (stored.IsEmpty()) {
      NOBIND_OBJECT_STORE_VERBOSE("already expired\n");
      return;
    }
    // Are we expiring the right object?
    if (stored == js) {
      NOBIND_OBJECT_STORE_VERBOSE("expiring\n");
      object_store.at(class_idx).erase(static_cast<T>(ptr));
    } else {
      NOBIND_OBJECT_STORE_VERBOSE("new object present\n");
    }
  }

  // We don't care for const, no two objects of the same type can have the same pointer anyway
  template <typename U> inline Napi::Value Get(size_t idx, const U *ptr) { return Get(idx, const_cast<U *>(ptr)); }
  template <typename U> inline void Put(size_t idx, const U *ptr, Napi::Value js) {
    return Put(idx, const_cast<U *>(ptr), js);
  }
  template <typename U> inline void Expire(size_t idx, const U *ptr, Napi::Value js) {
    Expire(idx, const_cast<U *>(ptr), js);
  }

  void Init(size_t s) { object_store.resize(s); }
};
} // namespace Nobind

#undef NOBIND_OBJECT_STORE_VERBOSE
