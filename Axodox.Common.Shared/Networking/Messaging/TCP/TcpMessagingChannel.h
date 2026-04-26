#pragma once
#include "Networking/Messaging/MessagingChannel.h"
#include "Networking/Sockets/Socket.h"
#include "Threading/BlockingCollection.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_messaging_channel final : public messaging_channel
  {
    inline static const Infrastructure::logger _logger{ "tcp_messaging_channel" };
    inline static const uint64_t _magic = 0x0123456789ABCDEF;

  public:
    tcp_messaging_channel(socket&& socket);
    virtual ~tcp_messaging_channel();

    virtual message_task send_message(std::vector<uint8_t>&& message) override;

  protected:
    virtual void on_opening() override;

  private:
    socket _socket;
    Threading::blocking_collection<std::shared_ptr<message_promise>> _messagesToSend;

    std::unique_ptr<Threading::background_thread> _sendThread;
    std::unique_ptr<Threading::background_thread> _receiveThread;

    void send_messages();
    void receive_messages();
  };
}
