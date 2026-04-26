#include "common_includes.h"
#include "TcpClient.h"

using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  tcp_client::tcp_client(socket&& socket) :
    _socket(move(socket))
  { }

  void tcp_client::connect(const socket_address& address)
  {
    _socket = { address.type(), socket_type::stream, ip_protocol::tcp };
    _socket.connect(address);
  }

  void tcp_client::close()
  {
    _socket.reset();
  }

  socket_stream tcp_client::stream()
  {
    return socket_stream(_socket);
  }

  const socket& tcp_client::client() const
  {
    return _socket;
  }
}