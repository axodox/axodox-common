#include "common_includes.h"
#include "TcpMessagingClient.h"
#include "TcpMessagingChannel.h"
#include <cstring>

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Networking
{
  tcp_messaging_client::tcp_messaging_client(const ip_endpoint& endpoint) :
    _endpoint(endpoint)
  { }

  const ip_endpoint& tcp_messaging_client::endpoint() const
  {
    return _endpoint;
  }

  unique_ptr<messaging_channel> tcp_messaging_client::get_channel()
  {
    socket_handle socket{ ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP) };
    if (socket == INVALID_SOCKET)
    {
      throw runtime_error("Failed to allocate TCP socket.");
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(_endpoint.hostname.c_str());
    address.sin_port = htons(_endpoint.port);
    if (::inet_pton(AF_INET, _endpoint.hostname.c_str(), &address.sin_addr) != 1)
    {
      throw runtime_error("Invalid IPv4 hostname.");
    }

    if (::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0)
    {
      _logger.log(log_severity::information, "Connected to {}:{}.", _endpoint.hostname, _endpoint.port);
      return make_unique<tcp_messaging_channel>(move(socket));
    }
    else
    {
      _logger.log(log_severity::warning, "Failed to connect to {}:{}.", _endpoint.hostname, _endpoint.port);
      throw runtime_error("Failed to connect to TCP server.");
    }
  }
}
