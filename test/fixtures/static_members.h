struct StaticMembers {
  int instance_member;
  static int static_member;
  static int static_readonly;

  StaticMembers();
  StaticMembers(int);
  int get_instance();
  static int get_static();
  static void nothing_static();
};
