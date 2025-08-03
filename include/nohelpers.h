#include "nonapi.h"
#include <functional>
#include <thread>

namespace Nobind {

/* ---------------------------------------------------------------------------
 * Run the tasklets waiting on the main thread queue
 * ---------------------------------------------------------------------------*/
template <typename T> void RunJob(Napi::Env env, Napi::Function callback, nullptr_t *, std::function<void()> *job) {
  auto env_data = env.GetInstanceData<T>();
  if (!env_data) {
    NOBIND_VERBOSE(OBJECT, "Skipping a finalizer because the environment has been shut down\n");
    return;
  }
  (*job)();
  // we do not want to block Node from exiting
  env_data->_Nobind_js_thread_jobs.Release();
}

/* ---------------------------------------------------------------------------
 * Schedule a job to run on the main thread
 * ---------------------------------------------------------------------------*/
template <typename T> void RunOnJSMainThread(Napi::BasicEnv env, std::function<void()> *job) {
  auto env_data = env.GetInstanceData<T>();
  if (!env_data) {
    // This is a very annoying quirk of Node.js/V8 - the
    // GC can run after the environment has been destroyed
    // Luckily, the behavior of Napi::Env::GetInstanceData
    // seems to be well defined - it returns nullptr
    NOBIND_VERBOSE(OBJECT, "Skipping a finalizer because the environment has been shut down\n");
    return;
  }
  if (env_data->_Nobind_js_thread == std::this_thread::get_id()) {
    // Run synchronously when called on the main thread
    (*job)();
    delete job;
  } else {
    env_data->_Nobind_js_thread_jobs.BlockingCall(job);
    env_data->_Nobind_js_thread_jobs.Acquire();
  }
}
}; // namespace Nobind
