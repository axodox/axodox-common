#pragma once
#include "Networking/Messaging/MessagingChannel.h"
#include "Networking/Sockets/SocketHandle.h"
#include "Threading/BlockingCollection.h"
#include "Threading/BackgroundThread.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API tcp_messaging_channel final : public messaging_channel
  {
    friend class tcp_messaging_server;
    friend class tcp_messaging_client;
    inline static const Infrastructure::logger _logger{ "tcp_messaging_channel" };
    inline static const uint64_t _magic = 0x0123456789ABCDEF;

  public:
    virtual ~tcp_messaging_channel();

    virtual message_task send_message(std::vector<uint8_t>&& message) override;

  protected:
    virtual void on_opening() override;

  private:
    socket_handle _socket;
    Threading::blocking_collection<std::shared_ptr<message_promise>> _messages_to_send;

    std::unique_ptr<Threading::background_thread> _send_thread;
    std::unique_ptr<Threading::background_thread> _receive_thread;

    tcp_messaging_channel(socket_handle&& socket);

    void send_worker();
    void receive_worker();
  };
}
