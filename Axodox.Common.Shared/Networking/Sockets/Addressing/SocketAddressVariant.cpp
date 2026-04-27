#include "common_includes.h"
#include "Infrastructure/BitwiseOperations.h"
#include "SocketAddressVariant.h"
#include "SocketAddressV4.h"
#include "SocketAddressV6.h"

using namespace Axodox::Infrastructure;
using namespace std;

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

  void socket_address_variant::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    auto kind = type();
    stream.write(kind);

    switch (kind)
    {
    case address_family::inet4:
      return as<socket_address_ipv4>()->serialize(stream, version);
    case address_family::inet6:
      return as<socket_address_ipv6>()->serialize(stream, version);
    default:
      throw logic_error("Cannot serialize this type");
    }
  }

  void socket_address_variant::deserialize(Storage::stream& stream, Storage::version_t version)
  {
    auto kind = stream.read<address_family>();

    switch (kind)
    {
    case address_family::inet4:
    {
      socket_address_ipv4 address;
      address.deserialize(stream, version);
      *this = address;
      break;
    }
    case address_family::inet6:
    {
      socket_address_ipv6 address;
      address.deserialize(stream, version);
      *this = address;
      break;
    }
    default:
      throw logic_error("Cannot deserialize this type");
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

  std::variant<std::monostate, ip_address_v4, ip_address_v6> socket_address_variant::address() const
  {
    switch (type())
    {
    case address_family::inet4:
      return as<socket_address_ipv4>()->address();
    case address_family::inet6:
      return as<socket_address_ipv6>()->address();
    default:
      return {};
    }
  }

  socket_address_variant::operator bool() const
  {
    return type() != address_family::unspecified;
  }
}