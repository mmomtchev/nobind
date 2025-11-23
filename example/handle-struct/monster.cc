#include "monster.h"

MonsterDefinition handle_monster(MonsterDefinition v) {
  std::printf("JS sent %s with %u eyes, %s\n", v.name.c_str(), v.eyes, v.greeter.Greet().c_str());
  v.eyes++;
  return v;
}

#include <nobind.h>

NOBIND_MODULE(handle_monster, m) {
  m.def<Hello>("Hello").cons<std::string &>().def<&Hello::Id>("id").def<&Hello::Greet>("greet");
  m.def<&handle_monster>("handleMonster");
}
