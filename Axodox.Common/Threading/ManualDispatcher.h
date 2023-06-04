#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  class AXODOX_COMMON_API manual_dispatcher
  {
  public:
    std::future<void> invoke_async(std::function<void()> callback);

    void process_pending_invocations();

  private:
    std::queue<std::packaged_task<void()>> _workItems;
    std::mutex _workMutex;
  };
}