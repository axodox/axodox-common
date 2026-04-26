#pragma once
#include "Networking/Messaging/MessagingClient.h"
#include "Infrastructure/ValuePtr.h"
#include "Networking/Sockets/Socket.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_messaging_client final : public messaging_client
  {
    inline static const Infrastructure::logger _logger{ "tcp_messaging_client" };

  public:
    tcp_messaging_client(const socket_address& endpoint);

    const socket_address_variant& endpoint() const;

  protected:
    virtual std::unique_ptr<messaging_channel> get_channel() override;

  private:
    socket_address_variant _endpoint;
  };
}
