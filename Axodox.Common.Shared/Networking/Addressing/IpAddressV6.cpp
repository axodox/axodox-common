#include "common_includes.h"
#include "IpAddressV6.h"

namespace Axodox::Networking
{
  const ip_address_v6 ip_address_v6::any{ 0, 0, 0, 0, 0, 0, 0, 0 };
  const ip_address_v6 ip_address_v6::loopback{ 0, 0, 0, 0, 0, 0, 0, 1 };
  const ip_address_v6 ip_address_v6::all_nodes_on_link{ 0xff02, 0, 0, 0, 0, 0, 0, 1 };

  std::optional<ip_address_v6> ip_address_v6::try_parse(const std::string_view text)
  {
    ip_address_v6 result;
    if (!inet_pton(AF_INET6, text.data(), &result)) return std::nullopt;
    return result;
  }

  ip_address_v6 ip_address_v6::from_in6_addr(in6_addr value)
  {
    ip_address_v6 result;
    auto& source = reinterpret_cast<const std::array<uint16_t, 8>&>(value);
    for (size_t i = 0; i < 8; i++)
    {
      result[i] = ntohs(source[i]);
    }
    return result;
  }

  in6_addr ip_address_v6::to_in6_addr() const
  {
    in6_addr result;
    auto& target = reinterpret_cast<std::array<uint16_t, 8>&>(result);
    for (size_t i = 0; i < 8; i++)
    {
      target[i] = htons(at(i));
    }
    return result;
  }
}