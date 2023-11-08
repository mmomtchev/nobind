#include <fixtures/static_members.h>

#include <nobind.h>

const char *version = "0.0.7";

NOBIND_MODULE(static_members, m) {
  m.def<&version, Nobind::ReadOnly>("version");
  m.def<StaticMembers>("StaticMembers")
    .cons<>()
    .def<&StaticMembers::get_instance>("get_instance")
    .def<&StaticMembers::get_static>("get_static")
    .def<&StaticMembers::static_member>("static_member")
    .def<&StaticMembers::static_member, Nobind::ReadOnly>("static_readonly")
    .def<&StaticMembers::nothing_static>("nothing_static");
}
