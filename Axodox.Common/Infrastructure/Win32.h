#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  struct win32_handle_traits
  {
    using type = T;

    static void close(type value) noexcept
    {
      CloseHandle(value);
    }

    static constexpr type invalid() noexcept
    {
      return nullptr;
    }
  };

  using window_handle = winrt::handle_type<win32_handle_traits<HWND>>;

  AXODOX_COMMON_API std::wstring to_wstring(std::string_view text);
}
#endif