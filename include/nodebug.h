#pragma once
#ifdef DEBUG
#include <cxxabi.h>
#include <string>

using namespace std::literals::string_literals;

namespace Nobind {

#define NOBIND_STR(x) #x ""

#define NOBIND_DEBUG_OPTS(V) V(OBJECT), V(STORE)

// Templated to be able to include in a header file
template <typename T> struct NobindDebug {
#define V(X) X
  enum debug_opt { NOBIND_DEBUG_OPTS(V) };
#undef V

#define V(X) NOBIND_STR(X)
  static constexpr const char *const debug_opt_names[] = {NOBIND_DEBUG_OPTS(V)};
#undef V

  static bool debug_opt_enabled[];

  static T Init() {
    for (size_t i = 0; i < sizeof(debug_opt_names) / sizeof(debug_opt_names[0]); i++) {
      std::string var_name = "NOBIND_DEBUG_"s + debug_opt_names[i];
      if (std::getenv(var_name.c_str())) {
        printf("%s debug enabled\n", debug_opt_names[i]);
        debug_opt_enabled[i] = true;
      } else {
        debug_opt_enabled[i] = false;
      }
    }
  }
  template <typename U> static std::string Demangle() { return Demangle(typeid(U).name()); }
  // https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
  static std::string Demangle(const char *name) {
    int status = -4;
    std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

    return (status == 0) ? res.get() : name;
  }
};

template struct Nobind::NobindDebug<void>;
template <typename T> bool Nobind::NobindDebug<T>::debug_opt_enabled[2];

#define NOBIND_INLINE
#define NOBIND_ASSERT(x) assert(x)
#define NOBIND_DEBUG_INIT Nobind::NobindDebug<void>::Init()

#define NOBIND_VERBOSE(sys, ...)                                                                                       \
  do {                                                                                                                 \
    if (NobindDebug<void>::debug_opt_enabled[NobindDebug<void>::debug_opt::sys])                                       \
      printf(__VA_ARGS__);                                                                                             \
  } while (0)

#define NOBIND_VERBOSE_TYPE(sys, T, FMT, ...)                                                                          \
  do {                                                                                                                 \
    if (NobindDebug<void>::debug_opt_enabled[NobindDebug<void>::debug_opt::sys]) {                                     \
      printf("[%s]" FMT, NobindDebug<void>::Demangle<T>().c_str(), __VA_ARGS__);                                       \
    }                                                                                                                  \
  } while (0)
} // namespace Nobind
#else

#define NOBIND_INLINE inline
#define NOBIND_ASSERT(x)
#define NOBIND_DEBUG_INIT
#define NOBIND_VERBOSE(...)
#define NOBIND_VERBOSE_TYPE(...)
#endif
