#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Threading
{
  template<typename T>
  std::future<T> threadpool_execute(std::function<T()>&& action)
  {
    struct threadpool_task
    {
      std::promise<T> promise;
      std::function<T()> action;
    };

    auto task = new threadpool_task();
    task->action = std::move(action);

    auto future = task->promise.get_future();
    winrt::Windows::System::Threading::ThreadPool::RunAsync([task](auto&&) {
      try
      {
        task->promise.set_value(task->action());
      }
      catch (...)
      {
        task->promise.set_exception(std::current_exception());
      }

      delete task;
    });

    return future;
  }
}
#endif