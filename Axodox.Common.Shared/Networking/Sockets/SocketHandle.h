#pragma once
#include "SocketHeaders.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API socket_handle
  {
  public:
    socket_handle() noexcept;
    explicit socket_handle(socket_t socket) noexcept;
    ~socket_handle() noexcept;

    socket_handle(const socket_handle&) = delete;
    socket_handle& operator=(const socket_handle&) = delete;

    socket_handle(socket_handle&& other) noexcept;
    socket_handle& operator=(socket_handle&& other) noexcept;

    void reset() noexcept;
    void attach(socket_t socket) noexcept;
    socket_t detach() noexcept;

    socket_t get() const noexcept;
    operator socket_t() const noexcept;
    explicit operator bool() const noexcept;

  private:
    socket_t _socket;
  };
}
