#pragma once
#include "Socket.h"
#include "TcpClient.h"
#include "Infrastructure/Events.h"
#include "Networking/Sockets/SocketStream.h"

namespace Axodox::Networking
{
  class tcp_client
  {
    Infrastructure::event_owner _events;

  public:
    tcp_client() = default;
    tcp_client(socket&& socket);

    tcp_client(tcp_client&&) noexcept = default;
    tcp_client& operator=(tcp_client&&) noexcept = default;

    void connect(const socket_address& address);
    void close();

    socket_stream stream();

    const socket& client() const;

  private:
    socket_address_variant _address;
    socket _socket;
  };
}