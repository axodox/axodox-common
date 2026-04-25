#include "common_includes.h"
#include "SocketStream.h"

using namespace std;

#ifdef PLATFORM_LINUX
using socket_len_t = size_t;
#endif

#ifdef PLATFORM_WINDOWS
using socket_len_t = int;
#endif

namespace {
  void check_result(int result)
  {
    if (result == 0)
    {
      throw runtime_error("Socket connection closed.");
    }

    if (result < 0)
    {
      throw runtime_error("Failed to send_message data over socket.");
    }
  }
}

namespace Axodox::Networking
{
  socket_stream::socket_stream(socket_t socket) :
    _socket(socket)
  {
    if (_socket == INVALID_SOCKET) throw runtime_error("Socket has been already closed.");
  }

  void socket_stream::write(span<const uint8_t> buffer)
  {
    size_t bytes_sent = 0;
    auto remaining = buffer.size();
    while (bytes_sent < buffer.size())
    {
      auto result = send(_socket, reinterpret_cast<const char*>(buffer.data() + bytes_sent), socket_len_t(remaining), 0);
      check_result(result);

      bytes_sent += size_t(result);
      remaining -= size_t(result);
    }
  }

  size_t socket_stream::read(span<uint8_t> buffer, bool partial)
  {
    size_t bytes_received = 0;
    auto remaining = buffer.size();
    while (bytes_received < buffer.size())
    {
      auto result = recv(_socket, reinterpret_cast<char*>(buffer.data() + bytes_received), socket_len_t(remaining), 0);
      check_result(result);

      bytes_received += size_t(result);
      remaining -= size_t(result);

      if (partial) break;
    }

    return bytes_received;
  }

  size_t socket_stream::position() const
  {
    throw logic_error("Cannot get position in network streams.");
  }

  void socket_stream::seek(size_t /*position*/)
  {
    throw logic_error("Cannot seek in network streams.");
  }

  size_t socket_stream::length() const
  {
    throw logic_error("Cannot get length of network streams.");
  }
}
