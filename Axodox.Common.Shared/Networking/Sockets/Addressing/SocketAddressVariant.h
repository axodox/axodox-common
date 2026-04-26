#pragma once
#include "SocketAddress.h"

namespace Axodox::Networking
{
  class socket_address_variant : public socket_address
  {
  public:
    static const address_family family = address_family::unspecified;

    socket_address_variant();
    explicit socket_address_variant(const socket_address& address);

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

    virtual std::string to_string() const;

  private:
    sockaddr_storage _address;
  };
}