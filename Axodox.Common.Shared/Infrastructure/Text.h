#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  AXODOX_COMMON_API std::string to_lower(std::string_view text);
  AXODOX_COMMON_API std::wstring to_lower(std::wstring_view text);

  AXODOX_COMMON_API std::vector<std::string_view> split(std::string_view text, char delimiter);
}