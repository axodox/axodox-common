#include "common_includes.h"
#include "TcpListener.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  tcp_listener::tcp_listener(uint16_t port) :
    tcp_listener(socket_address_ipv6{ ip_address_v6::any, port })
  { }

  tcp_listener::tcp_listener(const socket_address& address) :
    client_accepted(_events),
    _address(address)
  { }

  tcp_listener::~tcp_listener()
  {
    stop();
  }

  void tcp_listener::start(int backlog)
  {
    _socket = { _address.type(), socket_type::stream, ip_protocol::tcp };
    _socket.bind(_address);
    _socket.listen(backlog);

    _address = _socket.local_address();

    _worker = make_unique<background_thread>([this] { worker(); }, "* TCP listener thread - " + _address.to_string());
  }

  void tcp_listener::stop()
  {
    _socket.reset();
    _worker.reset();
  }

  const socket& tcp_listener::server() const
  {
    return _socket;
  }

  void tcp_listener::worker()
  {
    while (true)
    {
      auto socket = _socket.accept();
      if (!socket) return;

      tcp_client client{ move(socket) };
      _events.raise(client_accepted, this, move(client));
    }
  }
}