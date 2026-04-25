#include "common_includes.h"
#include "SocketHandle.h"

namespace Axodox::Networking
{
  socket_handle::socket_handle() noexcept :
    _socket(INVALID_SOCKET)
  { }

  socket_handle::socket_handle(socket_t socket) noexcept :
    _socket(socket)
  { }

  socket_handle::~socket_handle() noexcept
  {
    reset();
  }

  socket_handle::socket_handle(socket_handle&& other) noexcept :
    _socket(other._socket)
  {
    other._socket = INVALID_SOCKET;
  }

  socket_handle& socket_handle::operator=(socket_handle&& other) noexcept
  {
    if (this != &other)
    {
      reset();
      _socket = other._socket;
      other._socket = INVALID_SOCKET;
    }
    return *this;
  }

  void socket_handle::reset() noexcept
  {
    if (_socket != INVALID_SOCKET)
    {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
    }
  }

  void socket_handle::attach(socket_t socket) noexcept
  {
    reset();
    _socket = socket;
  }

  socket_t socket_handle::detach() noexcept
  {
    auto result = _socket;
    _socket = INVALID_SOCKET;
    return result;
  }

  socket_t socket_handle::get() const noexcept
  {
    return _socket;
  }

  socket_handle::operator socket_t() const noexcept
  {
    return _socket;
  }

  socket_handle::operator bool() const noexcept
  {
    return _socket != INVALID_SOCKET;
  }
}
