#pragma once
#include "pch.h"

namespace Axodox::Storage
{
#ifdef PLATFORM_WINDOWS
  AXODOX_COMMON_API std::filesystem::path app_folder();
#endif

  AXODOX_COMMON_API std::vector<uint8_t> read_file(const std::filesystem::path& path);
  AXODOX_COMMON_API std::vector<uint8_t> try_read_file(const std::filesystem::path& path);

  AXODOX_COMMON_API void write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);
  AXODOX_COMMON_API bool try_write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);

  AXODOX_COMMON_API std::optional<std::string> try_read_text(const std::filesystem::path& path);
  AXODOX_COMMON_API bool try_write_text(const std::filesystem::path& path, std::string_view text);
}