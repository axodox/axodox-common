#include "common_includes.h"
#include "Text.h"

using namespace std;

namespace
{
  constexpr char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  constexpr char base64_padding = '=';
}

namespace Axodox::Infrastructure
{
  std::string to_lower(std::string_view text)
  {
    string result{ text };
    transform(result.begin(), result.end(), result.begin(), [](char value) { return char(tolower(value)); });
    return result;
  }

  std::wstring to_lower(std::wstring_view text)
  {
    wstring result{ text };
    transform(result.begin(), result.end(), result.begin(), towlower);
    return result;
  }

  std::vector<std::string_view> split(std::string_view text, char delimiter)
  {
    vector<string_view> parts;

    const char* start = text.data();
    for (auto& character : text)
    {
      if (character == delimiter)
      {
        parts.push_back(string_view{ start, &character });
        start = &character + 1;
      }
    }

    if (start < text.data() + text.size())
    {
      parts.push_back(string_view{ start, text.data() + text.size() });
    }

    return parts;
  }

  std::string encode_base64(std::span<const uint8_t> data)
  {
    string result;
    result.reserve((data.size() + 2) / 3 * 4);

    size_t i = 0;
    while (i + 3 <= data.size())
    {
      uint32_t group = (uint32_t(data[i]) << 16) | (uint32_t(data[i + 1]) << 8) | uint32_t(data[i + 2]);
      result.push_back(base64_alphabet[(group >> 18) & 0x3f]);
      result.push_back(base64_alphabet[(group >> 12) & 0x3f]);
      result.push_back(base64_alphabet[(group >> 6) & 0x3f]);
      result.push_back(base64_alphabet[group & 0x3f]);
      i += 3;
    }

    auto remaining = data.size() - i;
    if (remaining == 1)
    {
      uint32_t group = uint32_t(data[i]) << 16;
      result.push_back(base64_alphabet[(group >> 18) & 0x3f]);
      result.push_back(base64_alphabet[(group >> 12) & 0x3f]);
      result.push_back(base64_padding);
      result.push_back(base64_padding);
    }
    else if (remaining == 2)
    {
      uint32_t group = (uint32_t(data[i]) << 16) | (uint32_t(data[i + 1]) << 8);
      result.push_back(base64_alphabet[(group >> 18) & 0x3f]);
      result.push_back(base64_alphabet[(group >> 12) & 0x3f]);
      result.push_back(base64_alphabet[(group >> 6) & 0x3f]);
      result.push_back(base64_padding);
    }

    return result;
  }

  bool try_decode_base64(std::string_view text, std::vector<uint8_t>& data)
  {
    static constexpr auto build_lookup = []() {
      array<int8_t, 256> lookup{};
      lookup.fill(-1);
      for (int8_t i = 0; i < int8_t(sizeof(base64_alphabet) - 1); ++i)
      {
        lookup[uint8_t(base64_alphabet[i])] = i;
      }
      return lookup;
    };
    static constexpr auto lookup = build_lookup();

    vector<uint8_t> result;
    result.reserve(text.size() / 4 * 3);

    uint32_t group = 0;
    int bits = 0;
    bool padding = false;
    for (auto character : text)
    {
      if (character == base64_padding)
      {
        padding = true;
        continue;
      }

      //No characters are allowed after padding.
      if (padding) return false;

      auto digit = lookup[uint8_t(character)];
      if (digit < 0) return false;

      group = (group << 6) | uint32_t(digit);
      bits += 6;
      if (bits >= 8)
      {
        bits -= 8;
        result.push_back(uint8_t((group >> bits) & 0xff));
      }
    }

    //Leftover bits must be zero, otherwise the input was malformed.
    if (bits > 0 && (group & ((1u << bits) - 1)) != 0) return false;

    data = move(result);
    return true;
  }
}