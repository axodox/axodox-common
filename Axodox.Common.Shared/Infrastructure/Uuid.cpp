#include "common_includes.h"
#include "Uuid.h"

using namespace std;

namespace {
  bool try_parse_hex(char character, uint8_t& value)
  {
    if (character >= '0' && character <= '9')
    {
      value = uint8_t(character - '0');
    }
    else if (character >= 'A' && character <= 'F')
    {
      value = uint8_t(10 + character - 'A');
    }
    else if (character >= 'a' && character <= 'f')
    {
      value = uint8_t(10 + character - 'a');
    }
    else
    {
      return false;
    }

    return true;
  }

  bool try_parse_guid(const string_view text, array<uint8_t, 16>& bytes)
  {
    if (text.size() != 36) return false;

    static size_t mapping[] = {
      3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15
    };

    auto isHigh = true;
    uint8_t accumulator = 0u;
    for (auto byteIndex = 0u, charIndex = 0u; auto c : text)
    {
      if (charIndex == 8 || charIndex == 13 || charIndex == 18 || charIndex == 23)
      {
        if (c != '-') return false;

        charIndex++;
        continue;
      }
      else
      {
        charIndex++;
      }

      uint8_t value;
      if (!try_parse_hex(c, value))
      {
        return false;
      }

      accumulator += isHigh ? value << 4 : value;

      if (isHigh)
      {
        isHigh = false;
      }
      else
      {
        bytes[mapping[byteIndex++]] = exchange(accumulator, 0u);
        isHigh = true;
      }
    }

    return true;
  }
}

namespace Axodox::Infrastructure
{
  uuid::uuid()
  {
    ranges::fill(bytes, 0u);
  }

  uuid::uuid(const std::string_view text)
  {
    if (!try_parse_guid(text, bytes))
    {
      throw invalid_argument("The provided value is not a valid GUID string.");
    }
  }

  uuid::operator std::string() const
  {
    return to_string();
  }

  std::string uuid::to_string() const
  {
    return std::format("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
      bytes[3], bytes[2], bytes[1], bytes[0], bytes[5], bytes[4], bytes[7], bytes[6],
      bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
  }

  std::optional<uuid> uuid::from_string(const std::string_view text)
  {
    uuid result;

    if (try_parse_guid(text, result.bytes))
    {
      return result;
    }
    else
    {
      return nullopt;
    }
  }
}
