#pragma once
#include <napi.h>
#include <noattributes.h>
#include <type_traits>

namespace Nobind {

namespace Typemap {
/**
 * Typemap::FromJS rules
 * - The constructor should check the incoming value
 * - The constructor is called on the V8 main thread
 * - operator* should return an in-place constructed prvalue
 * - operator* can be called in a background thread - no V8 Local<>s allowed
 * - The constructor can create state that will be destroyed after the function call
 */
template <typename T> class FromJS;

/**
 * Typemap::ToJS rules
 * - The constructor can be called in a background thread
 * - operator* should return an in-place constructed prvalue
 * - operator* will be called on the main V8 thread -> Local<>s allowed
 * - The constructor can create state that will be destroyed after the function call
 */
template <typename T, const ReturnAttribute &RETATTR = ReturnDefault> class ToJS;
} // namespace Typemap
namespace TypemapOverrides {

// Empty invalid overridden templates
template <typename T> class FromJS {
  FromJS() = delete;
};
template <typename T, const ReturnAttribute &RETATTR = ReturnDefault> class ToJS {
  ToJS() = delete;
};

} // namespace TypemapOverrides
} // namespace Nobind
