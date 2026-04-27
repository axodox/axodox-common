#include "common_includes.h"
#include "SocketStream.h"

using namespace std;

namespace Axodox::Networking
{
  socket_stream::socket_stream(socket& socket) :
    _socket(socket)
  {
    if (!_socket) throw runtime_error("Socket has been already closed.");
  }

  void socket_stream::write(span<const uint8_t> buffer)
  {
    size_t bytesSent = 0;
    while (bytesSent < buffer.size())
    {
      bytesSent += _socket.send(buffer.subspan(bytesSent));
    }
  }

  size_t socket_stream::read(span<uint8_t> buffer, bool partial)
  {
    size_t bytesReceived = 0;
    while (bytesReceived < buffer.size())
    {
      bytesReceived += _socket.receive(buffer.subspan(bytesReceived));
      if (partial) break;
    }

    return bytesReceived;
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
