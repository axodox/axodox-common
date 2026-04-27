#pragma once
#include "Networking/Sockets/UdpClient.h"

namespace Axodox::Networking
{
  struct service_address
  {
    std::string id;
    socket_address_variant address;
  };

  class AXODOX_COMMON_API service_locator
  {
    Infrastructure::event_owner _events;

  public:
    service_locator(const socket_address_variant& address);

    void locate_service(const std::string& id);
    Infrastructure::event_publisher<service_locator*, service_address> service_found;

  private:
    socket_address_variant _address;
    udp_client _client;

    Infrastructure::event_subscription _messageReceivedSubscription;

    void on_message_received(udp_client* sender, const udp_addressed_message& message);
  };
}