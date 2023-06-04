#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Threading
{
  AXODOX_COMMON_API void set_thread_name(const std::wstring& name);
  AXODOX_COMMON_API std::wstring get_thread_name();

  class AXODOX_COMMON_API thread_name_context
  {
  private:
    std::wstring _name;

  public:
    explicit thread_name_context(const std::wstring& name);
    ~thread_name_context();
  };
}
#endif