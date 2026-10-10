#pragma once
#ifdef PLATFORM_WINDOWS
#include "Networking/Messaging/MessagingChannel.h"
#include "Infrastructure/Logger.h"
#include "Threading/BlockingCollection.h"
#include "Threading/BackgroundThread.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Networking
{
  enum class named_pipe_end
  {
    server,
    client
  };

  //A messaging channel over a message-mode named pipe opened for overlapped I/O: one pipe message per message.
  class AXODOX_COMMON_API named_pipe_messaging_channel final : public messaging_channel
  {
    inline static const Infrastructure::logger _logger{ "named_pipe_messaging_channel" };

  public:
    named_pipe_messaging_channel(winrt::file_handle&& pipe, named_pipe_end end);
    ~named_pipe_messaging_channel();

    virtual message_task send_message(std::vector<uint8_t>&& message) override;

    //The process and session ID of the other end of the pipe.
    uint32_t peer_process_id() const;
    uint32_t peer_session_id() const;

  protected:
    virtual void on_opening() override;

  private:
    winrt::file_handle _pipe;
    winrt::handle _stopEvent;
    uint32_t _peerProcessId = 0;
    uint32_t _peerSessionId = 0;

    Threading::blocking_collection<std::shared_ptr<message_promise>> _messagesToSend;
    std::unique_ptr<Threading::background_thread> _sender;
    std::unique_ptr<Threading::background_thread> _receiver;

    void send_messages();
    void receive_messages();
    void close();
  };
}

#endif
#endif
