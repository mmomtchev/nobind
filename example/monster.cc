#include "monster.h"
#include "hello.h"
#include "monster_ptr.h"

MonsterDefinition handleMonster(MonsterDefinition v) {
  std::printf("JS sent %s with %u eyes, %s\n", v.name.c_str(), v.eyes, v.greeter.Greet().c_str());
  v.eyes++;
  return v;
}

void handleMonsterPtr(MonsterPtrDefinition v) {
  std::printf("JS sent %s with %u eyes, %s\n", v.name.c_str(), v.eyes, v.greeter->Greet().c_str());
}
