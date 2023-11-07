#pragma once
namespace Nobind {

struct Attribute {
  constexpr Attribute(){};
};

struct PropertyAttribute : public Attribute {
  enum Property { ReadWrite = 0, ReadOnly = 1 };
  Property value;
  constexpr PropertyAttribute() : value(ReadWrite) {}
  constexpr PropertyAttribute(Property v) : value(v) {}
  constexpr bool isReadOnly() const { return value == ReadOnly; }
};

/**
 * The property will be read only
 */
constexpr PropertyAttribute ReadOnly = PropertyAttribute(PropertyAttribute::ReadOnly);

/**
 * The property will be read-write (this is the default)
 */
constexpr PropertyAttribute ReadWrite = PropertyAttribute();

struct ReturnAttribute : public Attribute {
  enum Return { Shared, Owned, Default };
  Return value;
  constexpr ReturnAttribute() : value(Default) {}
  constexpr ReturnAttribute(Return v) : value(v) {}
  constexpr bool isShared() const { return value == Shared; }
  constexpr bool isOwned() const { return value == Owned; }
  template <bool DEFAULT> constexpr bool ShouldOwn() const {
    if (value == Shared) return false;
    if (value == Owned) return true;
    return DEFAULT;
  }
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
 * Returned pointers will be freed and returned references won't be freed (this is the default)
 */
constexpr ReturnAttribute ReturnDefault = ReturnAttribute();

struct ArgumentAttribute : public Attribute {
  constexpr ArgumentAttribute() {}
};

} // namespace Nobind
