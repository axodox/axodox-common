#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  AXODOX_COMMON_API std::string to_lower(std::string_view text);
  AXODOX_COMMON_API std::wstring to_lower(std::wstring_view text);

  AXODOX_COMMON_API std::vector<std::string_view> split(std::string_view text, char delimiter);

  //Encodes a binary buffer as a standard (RFC 4648) base64 string.
  AXODOX_COMMON_API std::string encode_base64(std::span<const uint8_t> data);

  //Decodes a standard (RFC 4648) base64 string into a binary buffer. Returns false on malformed input.
  AXODOX_COMMON_API bool try_decode_base64(std::string_view text, std::vector<uint8_t>& data);
}