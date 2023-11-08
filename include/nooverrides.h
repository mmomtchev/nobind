#pragma once
#include <napi.h>
#include <noattributes.h>
#include <type_traits>

namespace Nobind {
namespace TypemapOverrides {

// Empty invalid overridden templates
template <typename T> class FromJS {
  FromJS() = delete;
};
template <typename T, const ReturnAttribute &RETATTR = ReturnOwned> class ToJS {
  ToJS() = delete;
};

} // namespace TypemapOverrides
} // namespace Nobind
