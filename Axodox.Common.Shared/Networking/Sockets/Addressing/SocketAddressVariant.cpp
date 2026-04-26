#include "common_includes.h"
#include "Infrastructure/BitwiseOperations.h"
#include "SocketAddressVariant.h"
#include "SocketAddressV4.h"
#include "SocketAddressV6.h"

using namespace Axodox::Infrastructure;

namespace Axodox::Networking
{
  socket_address_variant::socket_address_variant()
  {
    zero_memory(_address);
  }

  socket_address_variant::socket_address_variant(const socket_address& address)
  {
    memcpy(&_address, address.get(), address.length());
  }

  address_family socket_address_variant::type() const
  {
    return address_family(_address.ss_family);
  }

  socklen_t socket_address_variant::length() const
  {
    return sizeof(_address);
  }

  sockaddr* socket_address_variant::get()
  {
    return reinterpret_cast<sockaddr*>(&_address);
  }

  std::string socket_address_variant::to_string() const
  {
    switch (type())
    {
    case address_family::inet4:
      return as<socket_address_ipv4>()->to_string();
    case address_family::inet6:
      return as<socket_address_ipv6>()->to_string();
    default:
      return "variant";
    }
  }

  uint16_t socket_address_variant::port() const
  {
    switch (type())
    {
    case address_family::inet4:
      return as<socket_address_ipv4>()->port();
    case address_family::inet6:
      return as<socket_address_ipv6>()->port();
    default:
      return 0;
    }
  }
}