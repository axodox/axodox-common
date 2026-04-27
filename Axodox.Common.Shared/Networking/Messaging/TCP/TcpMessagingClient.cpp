#include "common_includes.h"
#include "TcpMessagingClient.h"
#include "TcpMessagingChannel.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Networking
{
  tcp_messaging_client::tcp_messaging_client(const socket_address& endpoint) :
    _endpoint(endpoint)
  { }

  const socket_address_variant& tcp_messaging_client::endpoint() const
  {
    return _endpoint;
  }

  unique_ptr<messaging_channel> tcp_messaging_client::get_channel()
  {
    tcp_client socket;
    socket.connect(_endpoint);

    _logger.log(log_severity::information, "Connected to {}.", _endpoint.to_string());
    return make_unique<tcp_messaging_channel>(move(socket));
  }
}
