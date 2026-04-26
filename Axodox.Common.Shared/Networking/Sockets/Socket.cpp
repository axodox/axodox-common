#include "common_includes.h"
#include "Socket.h"

using namespace std;

namespace Axodox::Networking
{
  socket::socket() noexcept :
    _socket(INVALID_SOCKET)
  { }

  socket::socket(socket_t socket) noexcept :
    _socket(socket)
  { }

  socket::~socket() noexcept
  {
    reset();
  }

  socket::socket(socket_type type, ip_protocol protocol) : 
    socket(address_family::inet6, type, protocol)
  { }

  socket::socket(address_family family, socket_type type, ip_protocol protocol)
  {
    _socket = ::socket(int(family), int(type), int(protocol));
    if (_socket == INVALID_SOCKET) throw runtime_error("Failed to create socket. Reason: " + get_error_message());

    if (family == address_family::inet6) set_option(IPPROTO_IPV6, IPV6_V6ONLY, 0);
    set_io_mode(FIONBIO, 0);
  }

  socket::socket(socket&& other) noexcept :
    _socket(other._socket)
  {
    other._socket = INVALID_SOCKET;
  }

  socket& socket::operator=(socket&& other) noexcept
  {
    if (this != &other)
    {
      reset();
      _socket = other._socket;
      other._socket = INVALID_SOCKET;
    }
    return *this;
  }

  void socket::reset() noexcept
  {
    if (_socket != INVALID_SOCKET)
    {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
    }
  }

  void socket::attach(socket_t socket) noexcept
  {
    reset();
    _socket = socket;
  }

  socket_t socket::detach() noexcept
  {
    auto result = _socket;
    _socket = INVALID_SOCKET;
    return result;
  }

  socket_t socket::get() const noexcept
  {
    return _socket;
  }

  socket::operator socket_t() const noexcept
  {
    return _socket;
  }

  socket::operator bool() const noexcept
  {
    return _socket != INVALID_SOCKET;
  }

  size_t socket::receive(span<uint8_t> buffer)
  {
    ensure_socket();

    auto result = recv(_socket, reinterpret_cast<char*>(buffer.data()), socklen_t(buffer.size()), 0);
    if (result < 0) throw runtime_error("Failed to receive. Reason: " + get_error_message());
    return result;
  }

  size_t socket::send(span<const uint8_t> buffer)
  {
    ensure_socket();

    auto result = ::send(_socket, reinterpret_cast<const char*>(buffer.data()), socklen_t(buffer.size()), 0);
    if (result < 0) throw runtime_error("Failed to send. Reason: " + get_error_message());
    return result;
  }

  void socket::listen(int backlog)
  {
    ensure_socket();

    auto result = ::listen(_socket, backlog);
    if (result) throw runtime_error("Failed to start listening. Reason: " + get_error_message());
  }

  void socket::bind(const socket_address& address)
  {
    ensure_socket();

    auto l = address.length();
    auto result = ::bind(_socket, address.get(), address.length());
    if (result) throw runtime_error("Failed to bind socket. Reason: " + get_error_message());
  }

  void socket::connect(const socket_address& address)
  {
    ensure_socket();

    auto result = ::connect(_socket, address.get(), address.length());
    if (result) throw runtime_error("Failed to connect socket. Reason: " + get_error_message());
  }

  socket socket::accept(socket_address_variant* address)
  {
    ensure_socket();

    if (address)
    {
      auto addressLength = address->length();
      return socket{ ::accept(_socket, address->get(), &addressLength) };
    }
    else
    {
      return socket{ ::accept(_socket, nullptr, nullptr) };
    }
  }

  socket_address_variant socket::local_address() const
  {
    ensure_socket();

    socket_address_variant result;
    auto length = result.length();
    getsockname(_socket, result.get(), &length);
    return result;
  }

  socket_address_variant socket::remote_address() const
  {
    ensure_socket();

    socket_address_variant result;
    auto length = result.length();
    getpeername(_socket, result.get(), &length);
    return result;
  }

  void socket::set_io_mode(long command, u_long value)
  {
    ensure_socket();

    auto result = ioctlsocket(_socket, command, &value);
    if (result) throw logic_error("Failed to set socket option. Reason: " + get_error_message());
  }

  std::string socket::get_error_message()
  {
#ifdef PLATFORM_WINDOWS
    int error = WSAGetLastError();
    if (error == 0) return {};

    char message[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0, message, sizeof(message), nullptr);
    return message;
#else
    int error = errno;
    if (error == 0) return {};
    return std::strerror(error);
#endif
  }

  void socket::ensure_socket() const
  {
    if (_socket == INVALID_SOCKET) throw logic_error("The socket is invalid.");
  }
}
