#pragma once
#include "Networking/Messaging/MessagingServer.h"
#include "Networking/Sockets/Socket.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_messaging_server final : public messaging_server
  {
    inline static const Infrastructure::logger _logger{ "tcp_messaging_server" };

  public:
    tcp_messaging_server(uint16_t port);
    ~tcp_messaging_server();

    uint16_t port() const;

  protected:
    virtual void on_opening() override;

  private:
    uint16_t _port;
    socket _socket;
    std::unique_ptr<Threading::background_thread> _listenerThread;

    void listen_to_connections();
  };
}
