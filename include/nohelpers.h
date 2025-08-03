#include "nonapi.h"
#include <functional>
#include <thread>

// This standard construct (schedule a job to run on the
// main thread should probably be part of Node-API) in
// order to avoid relying on uv.h
// (but then again, isn't this Bun's own problem?)
#include <uv.h>

namespace Nobind {

/* ---------------------------------------------------------------------------
 * Run the tasklets waiting on the main thread queue
 * ---------------------------------------------------------------------------*/
template <typename T> void RunMainThreadQueue(uv_async_t *async) {
  auto env_data = reinterpret_cast<T *>(async->data);

  // As the lambdas are very light, it is better to not release the lock at all
  std::lock_guard<std::mutex> lock(env_data->_Nobind_js_thread_jobs_lock);
  while (!env_data->_Nobind_js_thread_jobs.empty()) {
    env_data->_Nobind_js_thread_jobs.front()();
    env_data->_Nobind_js_thread_jobs.pop();
  }
}

/* ---------------------------------------------------------------------------
 * Init the main thread tasklet queue
 * ---------------------------------------------------------------------------*/
template <typename T> void InitMainThreadQueue(Napi::Env env) {
  auto env_data = env.GetInstanceData<T>();
  uv_loop_t *event_loop;
  if (napi_get_uv_event_loop(env, &event_loop) != napi_ok)
    std::abort();
  if (uv_async_init(event_loop, &env_data->_Nobind_js_thread_async_handle, RunMainThreadQueue<T>) != 0)
    std::abort();
  // Do not block the event loop exit
  uv_unref(reinterpret_cast<uv_handle_t *>(&env_data->_Nobind_js_thread_async_handle));
  env_data->_Nobind_js_thread_async_handle.data = static_cast<void *>(env_data);
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
    if (uv_async_send(&env_data->_Nobind_js_thread_async_handle) != 0)
      std::abort();
  }
}
}; // namespace Nobind
