#include "common_includes.h"
#include "IpAddressV4.h"

namespace Axodox::Networking
{
  const ip_address_v4 ip_address_v4::any{ 0, 0, 0, 0 };
  const ip_address_v4 ip_address_v4::loopback{ 127, 0, 0, 1 };
  const ip_address_v4 ip_address_v4::broadcast{ 255, 255, 255, 255 };

  std::optional<ip_address_v4> Axodox::Networking::ip_address_v4::try_parse(const std::string_view text)
  {
    ip_address_v4 result;
    if (!inet_pton(AF_INET, text.data(), &result)) return std::nullopt;
    return result;
  }
  
  ip_address_v4 ip_address_v4::from_in_addr(in_addr value)
  {
    return reinterpret_cast<const ip_address_v4&>(value);
  }
  
  in_addr ip_address_v4::to_in_addr() const
  {
    return reinterpret_cast<const in_addr&>(*this);
  }
}