#pragma once
#ifdef PLATFORM_WINDOWS
#include "Networking/Messaging/MessagingClient.h"
#include "NamedPipeMessagingChannel.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Networking
{
  //A messaging client for a named_pipe_messaging_server; reconnects when the connection is lost.
  //Only connects to pipes created by the current user.
  class AXODOX_COMMON_API named_pipe_messaging_client final : public messaging_client
  {
    inline static const Infrastructure::logger _logger{ "named_pipe_messaging_client" };

  public:
    explicit named_pipe_messaging_client(std::string_view name);

    const std::string& name() const;

    //Connects to the pipe once, without retrying; throws if it cannot. The channel is returned unopened, so the caller
    //can subscribe to it first.
    static std::unique_ptr<named_pipe_messaging_channel> connect(std::string_view name);

  protected:
    virtual std::unique_ptr<messaging_channel> get_channel() override;

  private:
    std::string _name;
  };
}

#endif
#endif
