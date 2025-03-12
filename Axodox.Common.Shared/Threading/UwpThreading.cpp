#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Core_H
#include "UwpThreading.h"

using namespace winrt::Windows::UI::Core;

namespace Axodox::Threading
{
  void send_or_post(const CoreDispatcher& dispatcher, const std::function<void()>& action)
  {
    if (dispatcher.HasThreadAccess())
    {
      action();
    }
    else
    {
      dispatcher.RunAsync(CoreDispatcherPriority::Normal, action).get();
    }
  }
}
#endif
#endif