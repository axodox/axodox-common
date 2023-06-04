#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Storage
{
#ifdef WINRT_Windows_Storage_H
  AXODOX_COMMON_API std::vector<uint8_t> read_file(const winrt::Windows::Storage::StorageFile& file);
#endif
}
#endif