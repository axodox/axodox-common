#pragma once
#include "Networking/NetworkingPlatform.h"

namespace Axodox::Networking
{
  struct AXODOX_COMMON_API ip_address_v4 : std::array<uint8_t, 4>
  {
    static const ip_address_v4 any;
    static const ip_address_v4 loopback;
    static const ip_address_v4 broadcast;

    static std::optional<ip_address_v4> try_parse(const std::string_view text);

    static ip_address_v4 from_in_addr(in_addr value);

    in_addr to_in_addr() const;
  };
}