struct StaticMembers {
  int instance_member;
  static int static_member;

  StaticMembers();
  int get_instance();
  static int get_static();
};
