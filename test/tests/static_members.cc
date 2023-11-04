#include <fixtures/static_members.h>

#include "nobind.h"

NOBIND_MODULE(static_members, m) {
  m.def<StaticMembers>("StaticMembers")
    .cons<>()
    .def<&StaticMembers::get_instance>("get_instance")
    .def<&StaticMembers::get_static>("get_static")
    .def<&StaticMembers::static_member>("static_member");
}
