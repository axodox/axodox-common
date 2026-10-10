#pragma once
#ifdef PLATFORM_WINDOWS
#include "Networking/Messaging/MessagingServer.h"
#include "Infrastructure/Logger.h"
#include "Threading/BackgroundThread.h"
#include "NamedPipeSecurity.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Networking
{
  //A messaging server on a local named pipe which only the current user can open; remote clients are rejected.
  //Opening fails if a pipe with the same name already exists, so a server cannot be impersonated by starting first.
  class AXODOX_COMMON_API named_pipe_messaging_server final : public messaging_server
  {
    inline static const Infrastructure::logger _logger{ "named_pipe_messaging_server" };

  public:
    explicit named_pipe_messaging_server(std::string_view name);
    ~named_pipe_messaging_server();

    const std::string& name() const;

  protected:
    virtual void on_opening() override;

  private:
    std::string _name;
    named_pipe_security _security;
    winrt::handle _stopEvent;
    winrt::file_handle _waitingPipe;
    std::unique_ptr<Threading::background_thread> _listener;

    winrt::file_handle create_pipe(bool isFirstInstance);
    void accept_clients();
  };
}

#endif
#endif
