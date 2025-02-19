#include <nonapi.h>

#include <map>

namespace Nobind {
std::map<void *, Napi::Reference<Napi::Value>> object_store;
}
