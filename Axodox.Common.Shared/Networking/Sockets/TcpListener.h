#pragma once
#include "Socket.h"
#include "TcpClient.h"
#include "Infrastructure/Events.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_listener
  {
    Infrastructure::event_owner _events;

  public:
    tcp_listener(uint16_t port);
    tcp_listener(const socket_address& address);

    ~tcp_listener();

    tcp_listener(tcp_listener&&) noexcept = default;
    tcp_listener& operator=(tcp_listener&&) noexcept = default;

    void start(int backlog = SOMAXCONN);
    void stop();

    Infrastructure::event_publisher<tcp_listener*, tcp_client&&> client_accepted;

    const socket& server() const;

  private:
    socket_address_variant _address;
    socket _socket;

    std::unique_ptr<Threading::background_thread> _listener;

    void worker();
  };
}