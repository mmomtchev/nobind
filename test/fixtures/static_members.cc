#include "static_members.h"

StaticMembers::StaticMembers() : instance_member(1) {}

int StaticMembers::static_member = 2;
int StaticMembers::get_instance() { return instance_member; };
int StaticMembers::get_static() { return static_member; };
