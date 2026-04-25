#pragma once
#include "Networking/Messaging/MessagingClient.h"

namespace Axodox::Networking
{
  struct AXODOX_COMMON_API ip_endpoint
  {
    std::string hostname;
    uint16_t port = 0;
  };

  class AXODOX_COMMON_API tcp_messaging_client final : public messaging_client
  {
    inline static const Infrastructure::logger _logger{ "tcp_messaging_client" };

  public:
    tcp_messaging_client(const ip_endpoint& endpoint);

    const ip_endpoint& endpoint() const;

  protected:
    virtual std::unique_ptr<messaging_channel> get_client() override;

  private:
    ip_endpoint _endpoint;
  };
}
