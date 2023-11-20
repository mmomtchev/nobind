#include "string.h"

#include <napi.h>

using namespace Napi;

Napi::Number NapiStrlen(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (!info[0].IsString()) {
    throw Napi::TypeError::New(env, "Expected a string");
  }
  auto s = info[0].ToString().Utf8Value();
  auto r = Strlen(s);
  return Napi::Number::New(env, r);
}

class NapiString : public Napi::ObjectWrap<NapiString> {
  ::String *s;

public:
  NapiString(const Napi::CallbackInfo &info);
  ~NapiString();
  Napi::Value Strlen(const Napi::CallbackInfo &info);
  static Napi::Function GetClass(Napi::Env);
};

NapiString::NapiString(const Napi::CallbackInfo &info) : ObjectWrap(info) {
  Napi::Env env{info.Env()};

  if (!info[0].IsString()) {
    throw Napi::TypeError::New(env, "Expected a string");
  }

  s = new ::String(info[0].As<Napi::String>().Utf8Value());
}

NapiString::~NapiString() {
  delete s;
}

Napi::Value NapiString::Strlen(const Napi::CallbackInfo &info) {
  Napi::Env env{info.Env()};

  auto r = s->Len();
  return Napi::Number::New(env, r);
}

Napi::Function NapiString::GetClass(Napi::Env env) {
  return DefineClass(env, "String",
                     {
                         NapiString::InstanceMethod("length", &NapiString::Strlen),
                     });
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("String", NapiString::GetClass(env));
  exports.Set("strlen", Napi::Function::New(env, NapiStrlen));
  return exports;
}

NODE_API_MODULE(bench_napi_string, Init)
