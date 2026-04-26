#pragma once
#include "Addressing/SocketAddressVariant.h"

namespace Axodox::Networking
{
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
