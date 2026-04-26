#include "common_includes.h"
#include "SocketAddressV4.h"

namespace Axodox::Networking
{
  socket_address_ipv4::socket_address_ipv4()
  {
    _address = {};
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = 0;
  }

  socket_address_ipv4::socket_address_ipv4(const sockaddr& address)
  {
    memcpy(&_address, &address, sizeof(_address));
  }

  socket_address_ipv4::socket_address_ipv4(ip_address_v4 address, uint16_t port)
  {
    _address = {};
    _address.sin_family = AF_INET;
    _address.sin_addr = address.to_in_addr();
    _address.sin_port = htons(port);
  }

  address_family socket_address_ipv4::type() const
  {
    return address_family::inet4;
  }

  socklen_t socket_address_ipv4::length() const
  {
    return sizeof(_address);
  }

  sockaddr* socket_address_ipv4::get()
  {
    return reinterpret_cast<sockaddr*>(&_address);
  }

  ip_address_v4 socket_address_ipv4::address() const
  {
    return ip_address_v4::from_in_addr(_address.sin_addr);
  }

  void socket_address_ipv4::address(ip_address_v4 value)
  {
    _address.sin_addr = value.to_in_addr();
  }

  uint16_t socket_address_ipv4::port() const
  {
    return ntohs(_address.sin_port);
  }

  void socket_address_ipv4::port(uint16_t value)
  {
    _address.sin_port = htons(value);
  }

  std::string socket_address_ipv4::to_string() const
  {
    auto a = address();
    auto p = port();

    return std::format("{}.{}.{}.{}:{}", a[0], a[1], a[2], a[3], p);
  }
}