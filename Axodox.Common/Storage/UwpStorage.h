#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Storage
{
  AXODOX_COMMON_API std::vector<uint8_t> read_file(const winrt::Windows::Storage::StorageFile& file);
}
#endif