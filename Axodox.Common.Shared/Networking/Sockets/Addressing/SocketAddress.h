#pragma once
#include "Networking/NetworkingPlatform.h"

namespace Axodox::Networking
{
  enum class address_family
  {
    unspecified = AF_UNSPEC,
    inet4 = AF_INET,
    inet6 = AF_INET6
  };

  enum class socket_type
  {
    unspecified = 0,
    stream = SOCK_STREAM,
    datagram = SOCK_DGRAM,
    raw = SOCK_RAW
  };

  enum class ip_protocol
  {
    ipv4 = IPPROTO_IPV4,
    tcp = IPPROTO_TCP,
    udp = IPPROTO_UDP,
    ipv6 = IPPROTO_IPV6
  };

  struct socket_address
  {
    constexpr socket_address() = default;
    virtual ~socket_address() = default;

    virtual address_family type() const = 0;
    virtual socklen_t length() const = 0;
    virtual sockaddr* get() = 0;

    sockaddr& operator*()
    {
      return reinterpret_cast<sockaddr&>(*get());
    }

    const sockaddr& operator*() const
    {
      return reinterpret_cast<const sockaddr&>(*get());
    }

    const sockaddr* get() const
    {
      return const_cast<socket_address*>(this)->get();
    }

    virtual std::string to_string() const = 0;
  };
}