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
    _socket(socket_type::stream, ip_protocol::tcp)
  { }

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
    _socket.bind(socket_address_ipv6{ ip_address_v6::any, _port });

    auto address = _socket.local_address();
    if (_port == 0)
    {
      _port = address.port();
    }

    _socket.listen(10);

    _listenerThread = make_unique<background_thread>([this] { listen_to_connections(); }, "* TCP listener thread - " + address.to_string());
    _logger.log(log_severity::information, "Started listening for connections on port {}.", _port);
  }

  void tcp_messaging_server::listen_to_connections()
  {
    while (true)
    {
      auto client = _socket.accept();
      if (!client) return;

      on_client_connected(make_unique<tcp_messaging_channel>(move(client)));
    }
  }
}
