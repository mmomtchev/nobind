#include "string.h"

#include <napi.h>

using namespace Napi;

// strlen
Napi::Number NapiStrlen(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (!info[0].IsString()) {
    throw Napi::TypeError::New(env, "Expected a string");
  }
  auto s = info[0].ToString().Utf8Value();
  auto r = Strlen(s);
  return Napi::Number::New(env, r);
}

// class String
class NapiString : public Napi::ObjectWrap<NapiString> {
  ::String *s;

public:
  NapiString(const Napi::CallbackInfo &info);
  virtual ~NapiString();
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

NapiString::~NapiString() { delete s; }

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

// strlenAsync
class StrlenWorker : public Napi::AsyncWorker {
  Promise::Deferred deferred;
  const std::string s_;
  size_t result;

public:
  StrlenWorker(Napi::Env env, const std::string &s) : Napi::AsyncWorker(env), deferred(env), s_(s) {}
  virtual ~StrlenWorker() {}
  void Execute() override { result = Strlen(s_); }
  void OnOK() override {
    deferred.Resolve(Napi::Number::New(Env(), result));
  }
  Napi::Value GetPromise() {
    return deferred.Promise();
  }
};

Napi::Value NapiStrlenAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (!info[0].IsString()) {
    throw Napi::TypeError::New(env, "Expected a string");
  }
  auto s = info[0].ToString().Utf8Value();
  auto *worker = new StrlenWorker(env, s);
  worker->Queue();
  return worker->GetPromise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("String", NapiString::GetClass(env));
  exports.Set("strlen", Napi::Function::New(env, NapiStrlen));
  exports.Set("strlenAsync", Napi::Function::New(env, NapiStrlenAsync));
  return exports;
}

NODE_API_MODULE(bench_napi_string, Init)
