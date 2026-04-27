#pragma once
#include "Networking/Messaging/MessagingServer.h"
#include "Networking/Sockets/TcpListener.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_messaging_server final : public messaging_server
  {
    inline static const Infrastructure::logger _logger{ "tcp_messaging_server" };

  public:
    tcp_messaging_server(uint16_t port);

    uint16_t port() const;

  protected:
    virtual void on_opening() override;

  private:
    tcp_listener _listener;

    Infrastructure::event_subscription _clientAcceptedSubscription;
    void on_client_accepted(tcp_listener* listener, tcp_client&& client);
  };
}
