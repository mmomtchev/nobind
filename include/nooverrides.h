#pragma once
#include <noattributes.h>
#include <napi.h>

namespace Nobind {
namespace TypemapOverrides {

template <typename T> class FromJS {
public:
  static constexpr bool enable = false;
};

template <typename T, const ReturnAttribute &RETATTR> class ToJS {
public:
  static constexpr bool enable = false;
};

} // namespace TypemapOverrides
} // namespace Nobind
