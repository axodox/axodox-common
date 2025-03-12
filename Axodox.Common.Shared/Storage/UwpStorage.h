#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Storage
{
#ifdef WINRT_Windows_Storage_H
  AXODOX_COMMON_API std::vector<uint8_t> read_file(const winrt::Windows::Storage::StorageFile& file);

  AXODOX_COMMON_API winrt::Windows::Foundation::IAsyncAction read_files_recursively(winrt::Windows::Storage::StorageFolder const& folder, std::vector<winrt::Windows::Storage::StorageFile>& files);
#endif
}
#endif