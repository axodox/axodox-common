#pragma once
#include "SocketAddress.h"
#include "Networking/Addressing/IpAddressV6.h"

namespace Axodox::Networking
{
  class socket_address_ipv6 : public socket_address
  {
  public:
    static const address_family family = address_family::inet6;

    socket_address_ipv6();
    socket_address_ipv6(const sockaddr& address);
    socket_address_ipv6(ip_address_v6 address, uint16_t port);

    virtual address_family type() const;
    virtual socklen_t length() const;
    virtual sockaddr* get();

    ip_address_v6 address() const;
    void address(ip_address_v6 value);

    uint16_t port() const;
    void port(uint16_t value);

    virtual std::string to_string() const;

  private:
    sockaddr_in6 _address;
  };
}