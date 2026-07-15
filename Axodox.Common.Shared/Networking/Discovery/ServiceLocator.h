#pragma once
#include "Networking/Sockets/UdpClient.h"

namespace Axodox::Networking
{
  struct service_address
  {
    std::string id;
    socket_address_variant resolvedAddress;
    // The actual UDP sender address, distinct from `resolvedAddress` when the announced
    // address is loopback or otherwise not routed through the network stack.
    socket_address_variant senderAddress;
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

    static Axodox::Networking::socket_address_variant resolve_service_address(
      const Axodox::Networking::socket_address_variant& announced,
      const Axodox::Networking::socket_address_variant& sender);
  };
}