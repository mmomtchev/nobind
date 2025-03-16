#pragma once

namespace Nobind {

class Attribute {
public:
  constexpr Attribute() {};
};

class PropertyAttribute : public Attribute {
public:
  enum Property { ReadWrite = 0, ReadOnly = 1 };

  constexpr PropertyAttribute() : value(ReadWrite) {}
  constexpr PropertyAttribute(Property v) : value(v) {}
  constexpr bool isReadOnly() const { return value == ReadOnly; }

private:
  Property value;
};

/**
 * The property will be read only
 */
constexpr PropertyAttribute ReadOnly = PropertyAttribute(PropertyAttribute::ReadOnly);

/**
 * The property will be read-write (this is the default)
 */
constexpr PropertyAttribute ReadWrite = PropertyAttribute();

class ReturnAttribute : public Attribute {
public:
  enum Return { Shared = 0x1, Owned = 0x2, Nested = 0x40, Copy = 0x80 };
  enum Execution { Sync = 0x4, Async = 0x8 };
  enum Null { Allowed = 0x10, Forbidden = 0x20 };

  constexpr ReturnAttribute() : flags(0) {}
  constexpr ReturnAttribute(Return v) : flags(v) {}
  constexpr ReturnAttribute(Execution v) : flags(v) {}
  constexpr ReturnAttribute(Null v) : flags(v) {}
  constexpr ReturnAttribute operator|(const ReturnAttribute &other) const {
    return ReturnAttribute(flags | other.flags);
  }
  constexpr bool isShared() const { return (flags & Shared) == Shared; }
  constexpr bool isOwned() const { return (flags & Owned) == Owned; }
  constexpr bool isNested() const { return (flags & Nested) == Nested; }
  constexpr bool isCopy() const { return (flags & Copy) == Copy; }
  constexpr bool isReturnNullAccept() const { return (flags & Allowed) == Allowed; }
  constexpr bool isReturnNullThrow() const { return (flags & Forbidden) == Forbidden; }
  constexpr bool isAsync() const { return (flags & Async) == Async; }
  template <bool DEFAULT> constexpr bool ShouldOwn() const {
    if (isShared())
      return false;
    if (isOwned())
      return true;
    if (isNested())
      return false;
    if (isCopy())
      return true;
    return DEFAULT;
  }

private:
  constexpr ReturnAttribute(int v) : flags(v) {}
  int flags;
};

/**
 * The returned object will be freed when the GC destroys the JS proxy object
 */
constexpr ReturnAttribute ReturnOwned = ReturnAttribute(ReturnAttribute::Owned);

/**
 * The returned object won't be freed when the GC destroys the JS proxy object
 */
constexpr ReturnAttribute ReturnShared = ReturnAttribute(ReturnAttribute::Shared);

/**
 * Returned objects will be bound the to the lifespan of the parent, valid
 * only for class methods
 */
constexpr ReturnAttribute ReturnNested = ReturnAttribute(ReturnAttribute::Nested);

/**
 * Returned objects are to be copied, valid only for non-scalar objects
 */
constexpr ReturnAttribute ReturnCopy = ReturnAttribute(ReturnAttribute::Copy);

/**
 * Returned pointers will be freed and returned references won't be freed (this is the default)
 */
constexpr ReturnAttribute ReturnDefault = ReturnAttribute();

/**
 * The method will be synchronous (this is the default)
 */
constexpr ReturnAttribute ReturnSync = ReturnAttribute(ReturnAttribute::Sync);

/**
 * The method will be asynchronous
 */
constexpr ReturnAttribute ReturnAsync = ReturnAttribute(ReturnAttribute::Async);

/**
 * constexpr template to add ReturnAsync to a ReturnAttribute
 */
template <const ReturnAttribute &RET> constexpr ReturnAttribute RetWithAsync = Nobind::ReturnAsync | RET;

/**
 * This method can return nullptr without raising an exception
 */
constexpr ReturnAttribute ReturnNullAccept = ReturnAttribute(ReturnAttribute::Allowed);

/**
 * This method throws when it returns nullptr
 */
constexpr ReturnAttribute ReturnNullThrow = ReturnAttribute(ReturnAttribute::Forbidden);

class ArgumentAttribute : public Attribute {
public:
  constexpr ArgumentAttribute() {}
};

} // namespace Nobind
