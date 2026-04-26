#pragma once
#include "Networking/Sockets/UdpClient.h"
#include "Threading/LockedPtr.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API service_provider
  {
  public:
    using services_t = std::unordered_map<std::string, socket_address_variant>;

    service_provider(const socket_address_variant& address);

    void announce(const std::string& id, const socket_address_variant& address);

  private:
    mutable std::shared_mutex _mutex;
    socket_address_variant _address;
    udp_client _client;
    services_t _services;

    Infrastructure::event_subscription _messageReceivedSubscription;

    void on_message_received(udp_client* sender, const udp_addressed_message& message);

    void send_announcement(const socket_address& target, const std::string& id, const socket_address_variant& address);
  };
}