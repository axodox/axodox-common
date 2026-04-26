#include "common_includes.h"
#include "TcpMessagingServer.h"
#include "TcpMessagingChannel.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  tcp_messaging_server::tcp_messaging_server(uint16_t port) :
    _listener(port),
    _clientAcceptedSubscription(_listener.client_accepted({ this, &tcp_messaging_server::on_client_accepted }))
  { }

  uint16_t tcp_messaging_server::port() const
  {
    return _listener.server().local_address().port();
  }

  void tcp_messaging_server::on_opening()
  {
    _listener.start();
    _logger.log(log_severity::information, "Started listening for connections on port {}.", port());
  }

  void tcp_messaging_server::on_client_accepted(tcp_listener* /*listener*/, tcp_client&& client)
  {
    on_client_connected(make_unique<tcp_messaging_channel>(move(client)));
  }
}
