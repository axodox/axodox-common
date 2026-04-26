#include "common_includes.h"
#include "SocketAddressV6.h"

namespace Axodox::Networking
{
  socket_address_ipv6::socket_address_ipv6()
  {
    _address = {};
    _address.sin6_family = AF_INET6;
    _address.sin6_addr = IN6ADDR_ANY_INIT;
    _address.sin6_port = 0;
  }

  socket_address_ipv6::socket_address_ipv6(const sockaddr& address)
  {
    memcpy(&_address, &address, sizeof(_address));
  }

  socket_address_ipv6::socket_address_ipv6(ip_address_v6 address, uint16_t port)
  {
    _address = {};
    _address.sin6_family = AF_INET6;
    _address.sin6_addr = address.to_in6_addr();
    _address.sin6_port = htons(port);
  }

  address_family socket_address_ipv6::type() const
  {
    return address_family::inet6;
  }

  socklen_t socket_address_ipv6::length() const
  {
    return sizeof(_address);
  }

  sockaddr* socket_address_ipv6::get()
  {
    return reinterpret_cast<sockaddr*>(&_address);
  }

  ip_address_v6 socket_address_ipv6::address() const
  {
    return ip_address_v6::from_in6_addr(_address.sin6_addr);
  }

  void socket_address_ipv6::address(ip_address_v6 value)
  {
    _address.sin6_addr = value.to_in6_addr();
  }

  uint16_t socket_address_ipv6::port() const
  {
    return ntohs(_address.sin6_port);
  }

  void socket_address_ipv6::port(uint16_t value)
  {
    _address.sin6_port = htons(value);
  }

  std::string socket_address_ipv6::to_string() const
  {
    auto a = address();
    auto p = port();

    return std::format("[{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}]:{}",
      a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], p);
  }

  void socket_address_ipv6::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    stream.write(address());
    stream.write(port());
  }

  void socket_address_ipv6::deserialize(Storage::stream& stream, Storage::version_t version)
  {
    address(stream.read<ip_address_v6>());
    port(stream.read<uint16_t>());
  }
}