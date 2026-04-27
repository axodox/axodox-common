#pragma once
#include "Socket.h"
#include "Infrastructure/Events.h"
#include "Networking/Sockets/SocketStream.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  struct udp_addressed_message
  {
    socket_address_variant address;
    std::span<const uint8_t> content;
  };

  struct udp_options
  {
    bool is_address_reused = false;
    bool is_broadcast = false;
    std::variant<std::monostate, ip_address_v4, ip_address_v6> multicast_group = {};
  };

  class udp_client
  {
    Infrastructure::event_owner _events;

  public:
    udp_client(const udp_options& options = {});
    udp_client(uint16_t port, const udp_options& options = {});
    udp_client(const socket_address& address, const udp_options& options = {});

    ~udp_client();

    udp_client(udp_client&&) noexcept = default;
    udp_client& operator=(udp_client&&) noexcept = default;

    void connect(const socket_address& address);

    void send_message(std::span<const uint8_t> message);
    void send_message(const udp_addressed_message& message);
    Infrastructure::event_publisher<udp_client*, const udp_addressed_message&> message_received;

    const socket& client() const;

  private:
    socket _socket;

    std::unique_ptr<Threading::background_thread> _receiver;

    void receive();
  };
}