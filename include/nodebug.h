#pragma once
#ifdef DEBUG
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include <sstream>
#include <string>

using namespace std::literals::string_literals;

namespace Nobind {

template <const char *...OPTS> struct NobindDebug {
  static constexpr const char *const debug_opt_names[] = {OPTS...};
  static bool debug_opt_enabled[sizeof...(OPTS)];

  static void Init() {
    for (size_t i = 0; i < sizeof...(OPTS); i++) {
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
#ifndef _MSC_VER
    int status = -4;
    std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

    return (status == 0) ? res.get() : name;
#else
    return std::string{name};
#endif
  }
  template <const char *OPT, size_t... Ints> static inline bool Enabled(std::integer_sequence<size_t, Ints...>) {
    return (... || ((OPT == OPTS) && debug_opt_enabled[Ints]));
  }
  template <const char *OPT> static inline bool Enabled() {
    return Enabled<OPT>(std::make_integer_sequence<size_t, sizeof...(OPTS)>{});
  }
  template <const char *OPT, typename T> static void Log(T *obj, const char *fmt...) {
    if (Enabled<OPT>()) {
      va_list args;
      va_start(args, fmt);

      std::string type = Demangle<T>();

      std::ostringstream tid;
      tid << std::this_thread::get_id();

      std::printf("[%s : %p (tid %s)] ", type.c_str(), obj, tid.str().c_str());
      std::vprintf(fmt, args);
    }
  }
  template <const char *OPT> static void Log(const char *fmt...) {
    if (Enabled<OPT>()) {
      va_list args;
      va_start(args, fmt);
      std::vprintf(fmt, args);
    }
  }
};
template <const char *...OPTS> bool NobindDebug<OPTS...>::debug_opt_enabled[sizeof...(OPTS)];

// String literals as template arguments requires C++20
constexpr const char _nobind_debug_opt_STORE[] = "STORE";
constexpr const char _nobind_debug_opt_OBJECT[] = "OBJECT";
constexpr const char _nobind_debug_opt_LOCK[] = "LOCK";
using NobindDebugInstance = NobindDebug<_nobind_debug_opt_STORE, _nobind_debug_opt_OBJECT, _nobind_debug_opt_LOCK>;

#define NOBIND_INLINE
#define NOBIND_ASSERT(x) assert(x)
#define NOBIND_DEBUG_INIT Nobind::NobindDebugInstance::Init()

#define NOBIND_VERBOSE(SYS, ...) NobindDebugInstance::Log<_nobind_debug_opt_##SYS>(__VA_ARGS__)

#define NOBIND_VERBOSE_TYPE(SYS, T, O, ...) NobindDebugInstance::Log<_nobind_debug_opt_##SYS, T>(O, __VA_ARGS__)
} // namespace Nobind

#else

#define NOBIND_INLINE inline
#define NOBIND_ASSERT(x)
#define NOBIND_DEBUG_INIT
#define NOBIND_VERBOSE(...)
#define NOBIND_VERBOSE_TYPE(...)
#endif
