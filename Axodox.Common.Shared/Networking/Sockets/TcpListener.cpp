#include "common_includes.h"
#include "TcpListener.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"
#include "Infrastructure/Exceptions.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono_literals;

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
    _isStopping = false;
    _socket = { _address.type(), socket_type::stream, ip_protocol::tcp };
    _socket.bind(_address);
    _socket.listen(backlog);

    _address = _socket.local_address();

    _listener = make_unique<background_thread>([this] { worker(); }, "* TCP listener - " + _address.to_string());
  }

  void tcp_listener::stop()
  {
    //Set before closing the socket, which makes the pending accept fail
    _isStopping = true;
    _socket.reset();
    _listener.reset();
  }

  uint16_t tcp_listener::port() const
  {
    return _address.port();
  }

  const socket& tcp_listener::server() const
  {
    return _socket;
  }

  void tcp_listener::worker()
  {
    while (!_isStopping)
    {
      auto socket = _socket.accept();
      if (!socket)
      {
        if (_isStopping) return;

        //E.g. the client reset the connection before it was accepted, or the system is out of resources
        _logger.log(log_severity::warning, "Accepting a client failed.");
        this_thread::sleep_for(1s);
        continue;
      }

      try
      {
        tcp_client client{ move(socket) };
        _events.raise(client_accepted, this, move(client));
      }
      catch (...)
      {
        _logger.log(log_severity::error, "Setting up an accepted client failed. Reason: {}", get_exception_message());
      }
    }
  }
}