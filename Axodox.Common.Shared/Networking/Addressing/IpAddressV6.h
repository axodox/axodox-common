#pragma once
#include "Networking/NetworkingPlatform.h"

namespace Axodox::Networking
{
  struct AXODOX_COMMON_API ip_address_v6 : std::array<uint16_t, 8>
  {
    static const ip_address_v6 any;
    static const ip_address_v6 loopback;
    static const ip_address_v6 all_nodes_on_link;

    static std::optional<ip_address_v6> try_parse(const std::string_view text);

    static ip_address_v6 from_in6_addr(in6_addr value);

    in6_addr to_in6_addr() const;
  };
}