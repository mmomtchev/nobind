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

constexpr PropertyAttribute ReadOnly = PropertyAttribute(PropertyAttribute::ReadOnly);
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
constexpr ReturnAttribute ReturnOwned = ReturnAttribute(ReturnAttribute::Owned);
constexpr ReturnAttribute ReturnShared = ReturnAttribute(ReturnAttribute::Shared);
constexpr ReturnAttribute ReturnDefault = ReturnAttribute();

struct ArgumentAttribute : public Attribute {
  constexpr ArgumentAttribute() {}
};

} // namespace Nobind
