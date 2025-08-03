#include "nonapi.h"
#include <functional>
#include <thread>

namespace Nobind {

/* ---------------------------------------------------------------------------
 * Run the tasklets waiting on the main thread queue
 * ---------------------------------------------------------------------------*/
template <typename T> void RunMainThreadQueue(Napi::Env env, Napi::Function callback, nullptr_t *, void *) {
  auto env_data = env.GetInstanceData<T>();

  // As the lambdas are very light, it is better to not release the lock at all
  std::lock_guard<std::mutex> lock(env_data->_Nobind_js_thread_jobs_lock);
  while (!env_data->_Nobind_js_thread_jobs.empty()) {
    env_data->_Nobind_js_thread_jobs.front()();
    env_data->_Nobind_js_thread_jobs.pop();
  }
  // Disable the async because the queue is empty
  // (we do not want to block Node from exiting)
  env_data->_Nobind_tsfn.Release();
}

/* ---------------------------------------------------------------------------
 * Schedule a job to run on the main thread
 * ---------------------------------------------------------------------------*/
template <typename T> void RunOnJSMainThread(Napi::BasicEnv env, std::function<void()> &&job) {
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
    job();
  } else {
    // Normally the environment cannot be destroyed
    // with a waiting open uv_async, if was still alive above
    // it will still be alive when the tasklets run
    std::lock_guard<std::mutex> lock(env_data->_Nobind_js_thread_jobs_lock);
    env_data->_Nobind_js_thread_jobs.emplace(std::move(job));
    env_data->_Nobind_tsfn.BlockingCall();
    env_data->_Nobind_tsfn.Acquire();
  }
}
}; // namespace Nobind
