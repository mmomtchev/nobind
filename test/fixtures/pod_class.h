#include <type_traits>

struct IntObject {
  int value;
  IntObject() = default;
  IntObject(int);
  IntObject(const IntObject &) = default;
};

static_assert(std::is_pod_v<IntObject>, "IntObject is not a POD");
