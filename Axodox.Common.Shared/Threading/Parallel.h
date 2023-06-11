#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  AXODOX_COMMON_API void set_thread_name(std::string_view name);
  AXODOX_COMMON_API std::string get_thread_name();

  class AXODOX_COMMON_API thread_name_context
  {
  private:
    std::string _name;

  public:
    explicit thread_name_context(std::string_view name);
    ~thread_name_context();
  };
}