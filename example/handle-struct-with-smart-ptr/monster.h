#pragma once
#include "hello.h"
#include <memory>
#include <string>

// Type TypeScript type
const std::string MonsterDefinitionTSType = "{\n"
                                            "  name: string;\n"
                                            "  eyes: number;\n"
                                            "  fangs?: boolean;\n"
                                            "  feature: 'claws' | 'horn';\n"
                                            "  greeter: Hello;\n"
                                            "}";

struct MonsterDefinition {
  std::string name;
  unsigned eyes;
  bool fangs;
  enum Feature { CLAWS, HORN } feature;

  // C++ gets an actual smart pointer to the object held by JS
  // with automatic reference counting linked to the GC
  std::shared_ptr<Hello> greeter;
};

MonsterDefinition handleMonster(MonsterDefinition v);

//
// The custom typemaps
//

#include <nooverrides.h>

namespace Nobind {
namespace TypemapOverrides {
template <> class FromJS<MonsterDefinition> {
  MonsterDefinition val_;

public:
  inline explicit FromJS(Napi::Value val);
  inline MonsterDefinition Get() { return val_; }

  static const std::string TSType() { return MonsterDefinitionTSType; };
};

template <> class ToJS<MonsterDefinition> {
  Napi::Env env_;
  MonsterDefinition val_;

public:
  inline explicit ToJS(Napi::Env env, MonsterDefinition val) : env_(env), val_(val) {}
  inline Napi::Value Get();

  static const std::string TSType() { return MonsterDefinitionTSType; };
};

} // namespace TypemapOverrides
} // namespace Nobind

#include <nobind.h>

// Implement the parsing of the JS struct here
Nobind::TypemapOverrides::FromJS<MonsterDefinition>::FromJS(Napi::Value val) {
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
    val_.feature = MonsterDefinition::CLAWS;
  else if (str_feature == "horn")
    val_.feature = MonsterDefinition::HORN;
  else
    throw Napi::TypeError::New(env, std::string{"Invalid feature: "} + str_feature);

  val_.greeter = Nobind::Typemap::FromJS<std::shared_ptr<Hello>>(obj.Get("greeter")).Get();
}

// Implement the construction of the JS struct here
Napi::Value Nobind::TypemapOverrides::ToJS<MonsterDefinition>::Get() {
  Napi::EscapableHandleScope scope{env_};
  Napi::Object result = Napi::Object::New(env_);
  result.Set("name", Nobind::Typemap::ToJS<std::string>(env_, val_.name).Get());
  result.Set("eyes", Nobind::Typemap::ToJS<unsigned>(env_, val_.eyes).Get());
  result.Set("fangs", Nobind::Typemap::ToJS<bool>(env_, val_.fangs).Get());
  std::string feature = val_.feature == MonsterDefinition::CLAWS ? "claws" : "horn";
  result.Set("feature", Nobind::Typemap::ToJS<std::string>(env_, feature).Get());
  result.Set("greeter", Nobind::Typemap::ToJS<std::shared_ptr<Hello>>(env_, val_.greeter).Get());
  return scope.Escape(result);
}
