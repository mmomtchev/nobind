namespace Nobind {

struct arg {
  constexpr arg() {}
};

struct readOnly : public arg {
  constexpr readOnly() {}
};

namespace Internal {

// Does Attributes... contain readOnly?
template <typename... Attributes> constexpr bool isReadOnly() {
  if constexpr (sizeof...(Attributes) > 0) {
    return (std::is_same_v<Attributes, readOnly> || ...);
  } else {
    return false;
  }
}

}

} // namespace Nobind
