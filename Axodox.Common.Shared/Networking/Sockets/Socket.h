#pragma once
#include "SocketHeaders.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/ValuePtr.h"

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

    const sockaddr* get() const
    {
      return const_cast<socket_address*>(this)->get();
    }

    virtual std::string to_string() const = 0;
  };

  struct ip_address_v4 : std::array<uint8_t, 4>
  {
    static const ip_address_v4 any;
    static const ip_address_v4 loopback;
    static const ip_address_v4 broadcast;

    static std::optional<ip_address_v4> try_parse(const std::string_view text)
    {
      ip_address_v4 result;
      if (!inet_pton(AF_INET, text.data(), &result)) return std::nullopt;
      return result;
    }

    static ip_address_v4 from_in_addr(in_addr value)
    {
      return reinterpret_cast<const ip_address_v4&>(value);
    }

    in_addr to_in_addr() const
    {
      return reinterpret_cast<const in_addr&>(*this);
    }
  };

  inline const ip_address_v4 ip_address_v4::any{ 0, 0, 0, 0 };
  inline const ip_address_v4 ip_address_v4::loopback{ 127, 0, 0, 1 };
  inline const ip_address_v4 ip_address_v4::broadcast{ 255, 255, 255, 255 };

  class socket_address_ipv4 : public socket_address
  {
  public:
    static const address_family family = address_family::inet4;

    socket_address_ipv4()
    {
      _address = {};
      _address.sin_family = AF_INET;
      _address.sin_addr.s_addr = INADDR_ANY;
      _address.sin_port = 0;
    }

    socket_address_ipv4(const sockaddr& address)
    {
      memcpy(&_address, &address, sizeof(_address));
    }

    socket_address_ipv4(ip_address_v4 address, uint16_t port)
    {
      _address = {};
      _address.sin_family = AF_INET;
      _address.sin_addr = address.to_in_addr();
      _address.sin_port = htons(port);
    }

    virtual address_family type() const
    {
      return address_family::inet4;
    }

    virtual socklen_t length() const
    {
      return sizeof(_address);
    }

    virtual sockaddr* get()
    {
      return reinterpret_cast<sockaddr*>(&_address);
    }

    ip_address_v4 address() const
    {
      return ip_address_v4::from_in_addr(_address.sin_addr);
    }

    void address(ip_address_v4 value)
    {
      _address.sin_addr = value.to_in_addr();
    }

    uint16_t port() const
    {
      return ntohs(_address.sin_port);
    }

    void port(uint16_t value)
    {
      _address.sin_port = htons(value);
    }

    virtual std::string to_string() const
    {
      auto a = address();
      auto p = port();

      return std::format("{}.{}.{}.{}:{}", a[0], a[1], a[2], a[3], p);
    }

  private:
    sockaddr_in _address;
  };

  struct ip_address_v6 : std::array<uint16_t, 8>
  {
    static const ip_address_v6 any;
    static const ip_address_v6 loopback;
    static const ip_address_v6 all_nodes_on_link;
    
    static std::optional<ip_address_v6> try_parse(const std::string_view text)
    {
      ip_address_v6 result;
      if (!inet_pton(AF_INET6, text.data(), &result)) return std::nullopt;
      return result;
    }

    static ip_address_v6 from_in6_addr(in6_addr value)
    {
      ip_address_v6 result;
      auto& source = reinterpret_cast<const std::array<uint16_t, 8>&>(value);
      for (size_t i = 0; i < 8; i++)
      {
        result[i] = ntohs(source[i]);
      }
      return result;
    }

    in6_addr to_in6_addr() const
    {
      in6_addr result;
      auto& target = reinterpret_cast<std::array<uint16_t, 8>&>(result);
      for (size_t i = 0; i < 8; i++)
      {
        target[i] = htons(at(i));
      }
      return result;
    }
  };

  inline const ip_address_v6 ip_address_v6::any{ 0, 0, 0, 0, 0, 0, 0, 0 };
  inline const ip_address_v6 ip_address_v6::loopback{ 0, 0, 0, 0, 0, 0, 0, 1 };
  inline const ip_address_v6 ip_address_v6::all_nodes_on_link{ 0xff02, 0, 0, 0, 0, 0, 0, 1 };

  class socket_address_ipv6 : public socket_address
  {
  public:
    static const address_family family = address_family::inet6;

    socket_address_ipv6()
    {
      _address = {};
      _address.sin6_family = AF_INET6;
      _address.sin6_addr = IN6ADDR_ANY_INIT;
      _address.sin6_port = 0;
    }

    socket_address_ipv6(const sockaddr& address)
    {
      memcpy(&_address, &address, sizeof(_address));
    }

    socket_address_ipv6(ip_address_v6 address, uint16_t port)
    {
      _address = {};
      _address.sin6_family = AF_INET6;
      _address.sin6_addr = address.to_in6_addr();
      _address.sin6_port = htons(port);
    }

    virtual address_family type() const
    {
      return address_family::inet6;
    }

    virtual socklen_t length() const
    {
      return sizeof(_address);
    }

    virtual sockaddr* get()
    {
      return reinterpret_cast<sockaddr*>(&_address);
    }

    ip_address_v6 address() const
    {
      return ip_address_v6::from_in6_addr(_address.sin6_addr);
    }

    void address(ip_address_v6 value)
    {
      _address.sin6_addr = value.to_in6_addr();
    }

    uint16_t port() const
    {
      return ntohs(_address.sin6_port);
    }

    void port(uint16_t value)
    {
      _address.sin6_port = htons(value);
    }

    virtual std::string to_string() const
    {
      auto a = address();
      auto p = port();

      return std::format("[{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}]:{}", 
        a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], p);
    }

  private:
    sockaddr_in6 _address;
  };

  class socket_address_variant : public socket_address
  {
  public:
    static const address_family family = address_family::unspecified;

    socket_address_variant()
    {
      Infrastructure::zero_memory(_address);
    }

    explicit socket_address_variant(const socket_address& address)
    {
      memcpy(&_address, address.get(), address.length());
    }

    virtual address_family type() const
    {
      return address_family(_address.ss_family);
    }

    virtual socklen_t length() const
    {
      return sizeof(_address);
    }

    virtual sockaddr& operator*()
    {
      return reinterpret_cast<sockaddr&>(_address);
    }

    virtual const sockaddr& operator*() const
    {
      return reinterpret_cast<const sockaddr&>(_address);
    }

    virtual sockaddr* get()
    {
      return reinterpret_cast<sockaddr*>(&_address);
    }

    virtual const sockaddr* get() const
    {
      return reinterpret_cast<const sockaddr*>(&_address);
    }

    template<std::derived_from<socket_address> T>
      requires requires { { T::family } -> std::convertible_to<address_family>; }
    std::optional<T> as() const
    {
      if (T::family != type()) return std::nullopt;
      return T(operator*());
    }

    virtual std::string to_string() const
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

    uint16_t port() const
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

  private:
    sockaddr_storage _address;
  };

  class AXODOX_COMMON_API socket
  {
  public:
    socket() noexcept;
    explicit socket(socket_t socket) noexcept;
    ~socket() noexcept;

    socket(socket_type type, ip_protocol protocol);
    socket(address_family family, socket_type type, ip_protocol protocol);

    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;

    socket(socket&& other) noexcept;
    socket& operator=(socket&& other) noexcept;

    void reset() noexcept;
    void attach(socket_t socket) noexcept;
    socket_t detach() noexcept;

    socket_t get() const noexcept;
    operator socket_t() const noexcept;
    explicit operator bool() const noexcept;

    size_t receive(std::span<uint8_t> buffer);
    size_t send(std::span<const uint8_t> buffer);

    void listen(int backlog);
    void bind(const socket_address& address);
    void connect(const socket_address& address);
    socket accept(socket_address_variant* address = nullptr);

    socket_address_variant local_address() const;
    socket_address_variant remote_address() const;

    template<typename T>
    void set_option(int level, int option, T value)
    {
      auto result = setsockopt(_socket, level, option, reinterpret_cast<const char*>(&value), sizeof(T));
      if (result) throw std::logic_error("Failed to set socket option.");
    }

    void set_io_mode(long command, u_long value);

  private:
    socket_t _socket;

    std::string get_error_message();
    void ensure_socket() const;
  };
}
