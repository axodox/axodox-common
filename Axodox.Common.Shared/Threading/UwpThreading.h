#pragma once
#ifdef PLATFORM_WINDOWS
#ifdef WINRT_Windows_UI_Core_H
#include "common_includes.h"

namespace Axodox::Threading
{
  AXODOX_COMMON_API void send_or_post(const winrt::Windows::UI::Core::CoreDispatcher& dispatcher, const std::function<void()>& action);

  typedef winrt::Windows::Foundation::IAsyncAction async_action;
}
#endif
#endif