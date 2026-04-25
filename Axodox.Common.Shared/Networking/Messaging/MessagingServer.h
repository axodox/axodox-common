#pragma once
#include "MessagingChannel.h"
#include "Infrastructure/Openable.h"
#include "Threading/LockedPtr.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API messaging_server : public Infrastructure::openable
  {
    Infrastructure::event_owner _events;

  public:
    messaging_server();
    ~messaging_server();

    messaging_server(const messaging_server&) = delete;
    messaging_server& operator=(const messaging_server&) = delete;

    uint32_t client_count() const;
    Threading::locked_ptr<std::vector<std::unique_ptr<messaging_channel>>> clients();

    void broadcast(std::vector<uint8_t>&& message, messaging_channel* exception = nullptr);

    Infrastructure::event_publisher<messaging_server*, messaging_channel*> client_connected;
    Infrastructure::event_publisher<messaging_channel*, std::span<const uint8_t>> message_received;
    Infrastructure::event_publisher<messaging_server*, messaging_channel*> client_disconnected;

  protected:
    void on_client_connected(std::unique_ptr<messaging_channel>&& channel);

  private:
    std::shared_mutex _mutex;
    std::vector<std::unique_ptr<messaging_channel>> _clients;
    std::unique_ptr<messaging_channel> _clientParkingSpace;
  };
}
