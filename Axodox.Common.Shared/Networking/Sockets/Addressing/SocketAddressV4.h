#pragma once
#include "SocketAddress.h"
#include "Networking/Addressing/IpAddressV4.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API socket_address_ipv4 : public socket_address
  {
  public:
    static const address_family family = address_family::inet4;

    socket_address_ipv4();
    socket_address_ipv4(const sockaddr& address);
    socket_address_ipv4(ip_address_v4 address, uint16_t port);

    virtual address_family type() const;
    virtual socklen_t length() const;
    virtual sockaddr* get();

    ip_address_v4 address() const;
    void address(ip_address_v4 value);

    uint16_t port() const;
    void port(uint16_t value);

    virtual std::string to_string() const;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);

  private:
    sockaddr_in _address;
  };
}