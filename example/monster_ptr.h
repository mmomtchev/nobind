#pragma once
#include "hello.h"
#include "monster.h"
#include <string>

// Type TypeScript type is the same as the other example

struct MonsterPtrDefinition {
  std::string name;
  unsigned eyes;
  bool fangs;
  enum Feature { CLAWS, HORN } feature;

  // Get a pointer to the actual JS object
  Hello *greeter;
  // This is needed to protect the above object from the GC
  // when this pointer will be kept on the C++ side. This
  // renders the structure non-copyconstructible. The JS
  // object will be preserved as long as this exists.
  //
  // You don't need this part if the pointer won't be used
  // after the function call, nobind protects it automatically
  Napi::ObjectReference greeter_gc_guard;
};

void handleMonsterPtr(MonsterPtrDefinition v);

//
// The custom typemaps
//

#include <nooverrides.h>

namespace Nobind {
namespace TypemapOverrides {
template <> class FromJS<MonsterPtrDefinition> {
  MonsterPtrDefinition val_;

public:
  inline explicit FromJS(Napi::Value val);
  inline MonsterPtrDefinition Get() { return std::move(val_); }

  static const std::string TSType() { return MonsterDefinitionTSType; };
};

} // namespace TypemapOverrides
} // namespace Nobind

#include <nobind.h>

// Implement the parsing of the JS struct here
Nobind::TypemapOverrides::FromJS<MonsterPtrDefinition>::FromJS(Napi::Value val) {
  Napi::Env env{val.Env()};

  if (!val.IsObject()) {
    throw Napi::TypeError::New(val.Env(), "Expected an object");
  }
  Napi::Object obj = val.ToObject();

  val_.name = Nobind::Typemap::FromJS<std::string>(obj.Get("name")).Get();
  val_.eyes = Nobind::Typemap::FromJS<unsigned>(obj.Get("eyes")).Get();

  Napi::Value js_fangs = obj.Get("fangs");
  if (js_fangs.IsUndefined()) {
    val_.fangs = false;
  } else {
    val_.fangs = Nobind::Typemap::FromJS<bool>(js_fangs).Get();
  }

  Napi::Value js_feature = obj.Get("feature");
  std::string str_feature = Nobind::Typemap::FromJS<std::string>(js_feature).Get();
  if (str_feature == "claws")
    val_.feature = MonsterPtrDefinition::CLAWS;
  else if (str_feature == "horn")
    val_.feature = MonsterPtrDefinition::HORN;
  else
    throw Napi::TypeError::New(env, std::string{"Invalid feature: "} + str_feature);

  val_.greeter = Nobind::Typemap::FromJS<Hello *>(obj.Get("greeter")).Get();
  val_.greeter_gc_guard = Napi::Persistent(obj.Get("greeter").ToObject());
}
