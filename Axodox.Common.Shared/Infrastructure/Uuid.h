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
  };
}