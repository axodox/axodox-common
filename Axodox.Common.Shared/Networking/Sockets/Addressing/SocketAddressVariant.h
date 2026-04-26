#pragma once
#include "SocketAddress.h"
#include "Networking/Addressing/IpAddressV4.h"
#include "Networking/Addressing/IpAddressV6.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API socket_address_variant : public socket_address
  {
  public:
    static const address_family family = address_family::unspecified;

    socket_address_variant();
    socket_address_variant(const socket_address& address);

    virtual address_family type() const;
    virtual socklen_t length() const;
    virtual sockaddr* get();

    template<std::derived_from<socket_address> T>
      requires requires { { T::family } -> std::convertible_to<address_family>; }
    std::optional<T> as() const
    {
      if (T::family != type()) return std::nullopt;
      return T(operator*());
    }

    uint16_t port() const;
    std::variant<std::monostate, ip_address_v4, ip_address_v6> address() const;

    operator bool() const;
    virtual std::string to_string() const;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);

  private:
    sockaddr_storage _address;
  };
}