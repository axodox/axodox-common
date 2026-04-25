#include "common_includes.h"
#include "TcpMessagingServer.h"
#include "TcpMessagingChannel.h"
#include <stdexcept>
#include <cstring>

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  tcp_messaging_server::tcp_messaging_server(uint16_t port) :
    _port(port),
    _socket(socket(AF_INET, SOCK_STREAM, IPPROTO_IP))
  {
    if (!_socket)
    {
      throw runtime_error("Failed to allocate TCP listener socket.");
    }
    u_long value = 0;
    ioctlsocket(_socket, FIONBIO, &value);
  }

  tcp_messaging_server::~tcp_messaging_server()
  {
    _socket.reset();
    _listenerThread.reset();
  }

  uint16_t tcp_messaging_server::port() const
  {
    return _port;
  }

  void tcp_messaging_server::on_opening()
  {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);
    memset(address.sin_zero, 0, sizeof(address.sin_zero));

    auto result = ::bind(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
    if (result != 0) throw runtime_error("Cannot bind TCP listener.");

    if (_port == 0)
    {
      sockaddr_in bound{};
      socklen_t len = sizeof(bound);
      if (::getsockname(_socket, reinterpret_cast<sockaddr*>(&bound), &len) == 0)
      {
        _port = ntohs(bound.sin_port);
      }
    }

    result = listen(_socket, 10);
    if (result != 0) throw runtime_error("Cannot start TCP listener.");

    _listenerThread = make_unique<background_thread>([this] { listen_to_connections(); }, "* TCP listener thread");
    _logger.log(log_severity::information, "Started listening for connections on port {}.", _port);
  }

  void tcp_messaging_server::listen_to_connections()
  {
    while (true)
    {
      sockaddr_in address{};
      socklen_t addrlen{ sizeof(address) };
      socket_handle client{ accept(_socket, reinterpret_cast<sockaddr*>(&address), &addrlen) };

      if (!client) return;

      on_client_connected(unique_ptr<messaging_channel>{ new tcp_messaging_channel(move(client)) });
    }
  }
}
