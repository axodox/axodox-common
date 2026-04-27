#pragma once
#include "MessagingChannel.h"
#include "Infrastructure/Logger.h"
#include "Threading/BackgroundThread.h"
#include "Threading/Events.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API messaging_client : public Infrastructure::openable
  {
    inline static const Infrastructure::logger _logger{ "messaging_client" };
    Infrastructure::event_owner _events;

  public:
    messaging_client();
    ~messaging_client();

    Infrastructure::event_publisher<messaging_client*, messaging_channel*> connected;
    Infrastructure::event_publisher<messaging_channel*, std::span<const uint8_t>> message_received;
    Infrastructure::event_publisher<messaging_client*, messaging_channel*> disconnected;

    message_task send_message(std::vector<uint8_t>&& message);

    bool is_connected() const;

  protected:
    virtual void on_opening() override;
    virtual std::unique_ptr<messaging_channel> get_channel() = 0;

  private:
    mutable std::mutex _mutex;
    std::unique_ptr<messaging_channel> _channel;
    std::unique_ptr<Threading::background_thread> _connectionThread;

    bool _isShuttingDown = false;
    Threading::manual_reset_event _wakeupEvent;

    void connect();
  };
}
