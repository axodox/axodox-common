#pragma once
#include "common_includes.h"

namespace Axodox::Storage
{
#ifdef PLATFORM_WINDOWS
  AXODOX_COMMON_API std::filesystem::path app_folder();

  inline std::filesystem::path lib_folder() //Do not move out of header - it won't work that way
  {
    HMODULE moduleHandle;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"", &moduleHandle);

    std::wstring filePath(MAX_PATH, L'\0');
    GetModuleFileName(moduleHandle, filePath.data(), (DWORD)filePath.size());
    return std::filesystem::path(filePath).parent_path();
  }
#endif

  AXODOX_COMMON_API std::vector<uint8_t> read_file(const std::filesystem::path& path);
  AXODOX_COMMON_API std::vector<uint8_t> try_read_file(const std::filesystem::path& path);

  AXODOX_COMMON_API void write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);
  AXODOX_COMMON_API bool try_write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);

  AXODOX_COMMON_API std::optional<std::string> try_read_text(const std::filesystem::path& path);
  AXODOX_COMMON_API bool try_write_text(const std::filesystem::path& path, std::string_view text);
}