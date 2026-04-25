#pragma once
#include "Infrastructure/Events.h"
#include "Infrastructure/Openable.h"
#include "MessageTask.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API messaging_channel : public Infrastructure::openable
  {
    friend class messaging_server;
    friend class messaging_client;
    Infrastructure::event_owner _events;

  public:
    messaging_channel();

    messaging_channel(const messaging_channel&) = delete;
    messaging_channel& operator=(const messaging_channel&) = delete;

    bool is_connected() const;

    virtual message_task send_message(std::vector<uint8_t>&& message) = 0;

    Infrastructure::event_publisher<messaging_channel*, std::span<const uint8_t>> message_received;
    Infrastructure::event_publisher<messaging_channel*> disconnected;

  protected:
    void on_received(std::span<const uint8_t> message);
    void on_disconnected();

  private:
    std::mutex _mutex;
    bool _is_connected = true;
  };
}
