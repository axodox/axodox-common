#pragma once
#include "BitwiseOperations.h"

namespace Axodox::Infrastructure
{
  struct AXODOX_COMMON_API uuid
  {
    std::array<uint8_t, 16> bytes;

    uuid();

    uuid(const std::string_view text);

    operator std::string() const;

    std::string to_string() const;

    static std::optional<uuid> from_string(const std::string_view text);

    bool operator==(const uuid& other) const = default;
  };
}

template<>
struct std::formatter<Axodox::Infrastructure::uuid, char> : std::formatter<std::string_view, char>
{
  template<typename context_t>
  auto format(const Axodox::Infrastructure::uuid& value, context_t& context) const
  {
    return std::formatter<std::string_view, char>::format(value.to_string(), context);
  }
};

template<>
struct std::formatter<Axodox::Infrastructure::uuid, wchar_t> : std::formatter<std::wstring_view, wchar_t>
{
  template<typename context_t>
  auto format(const Axodox::Infrastructure::uuid& value, context_t& context) const
  {
    //The text of a uuid is hexadecimal digits and dashes, so it is always ASCII.
    auto text = value.to_string();
    return std::formatter<std::wstring_view, wchar_t>::format(std::wstring(text.begin(), text.end()), context);
  }
};