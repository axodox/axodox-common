#pragma once
#include "Socket.h"
#include "TcpClient.h"
#include "Infrastructure/Events.h"
#include "Infrastructure/Logger.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_listener
  {
    inline static const Infrastructure::logger _logger{ "tcp_listener" };
    Infrastructure::event_owner _events;

  public:
    tcp_listener(uint16_t port);
    tcp_listener(const socket_address& address);

    ~tcp_listener();

    tcp_listener(tcp_listener&&) noexcept = default;
    tcp_listener& operator=(tcp_listener&&) noexcept = default;

    void start(int backlog = SOMAXCONN);
    void stop();

    uint16_t port() const;

    Infrastructure::event_publisher<tcp_listener*, tcp_client&&> client_accepted;

    const socket& server() const;

  private:
    socket_address_variant _address;
    socket _socket;
    bool _isStopping = false;

    std::unique_ptr<Threading::background_thread> _listener;

    void worker();
  };
}