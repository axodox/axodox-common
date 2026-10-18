#pragma once
#ifdef PLATFORM_WINDOWS
#include "Networking/Messaging/MessagingChannel.h"
#include "Infrastructure/Logger.h"
#include "Threading/BlockingCollection.h"
#include "Threading/BackgroundThread.h"
#include "Threading/Win32Events.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Networking
{
  enum class named_pipe_end
  {
    server,
    client
  };

  //The process and session ID of the other end of a pipe.
  struct named_pipe_peer_description
  {
    uint32_t process_id = 0;
    uint32_t session_id = 0;
  };

  //A messaging channel over a message-mode named pipe opened for overlapped I/O: one pipe message per message.
  class AXODOX_COMMON_API named_pipe_messaging_channel final : public messaging_channel
  {
    inline static const Infrastructure::logger _logger{ "named_pipe_messaging_channel" };

  public:
    named_pipe_messaging_channel(winrt::file_handle&& pipe, named_pipe_end end);
    ~named_pipe_messaging_channel();

    virtual message_task send_message(std::vector<uint8_t>&& message) override;

    named_pipe_peer_description remote_peer() const;

    //The full path of a named pipe: \\.\pipe\<name>
    static std::wstring pipe_path(std::string_view name);

  protected:
    virtual void on_opening() override;

  private:
    winrt::file_handle _pipe;
    Threading::win32_manual_reset_event _stopEvent;
    named_pipe_peer_description _remotePeer;

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
