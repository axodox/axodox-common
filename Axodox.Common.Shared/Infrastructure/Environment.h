#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  AXODOX_COMMON_API std::string get_environment_variable(std::string_view name);
}

#endif